import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { DxLibTaskProvider, runBuild, TASK_TYPE } from './build/taskProvider';
import { CPPTOOLS_ID, cpptoolsInstalled, intelliSenseState, warnIfCpptoolsMissing } from './env/cpptools';
import { registerHlslFormatter } from './format/hlslFormatter';
import { collectEnvironment, currentFolder, getConfig, isDxLibProject, SDK_FIX_HINT, sdkProblem, setGlobalConfig } from './env/environment';
import { inspectSdk } from './env/sdk';
import { detectVisualStudio, installerSetupPath, workloadInstallArgs } from './env/vswhere';
import { invalidateDxLibIndex, registerDxLibHover } from './hover/dxlibHover';
import { DxLibConfigurationProvider } from './intellisense/configProvider';
import { DxLibPanelProvider } from './panel/panelView';
import { createProject, CreateProjectArgs } from './project/createProject';
import { saveAsTemplateWork, SaveTemplateArgs } from './project/saveAsTemplate';
import { listTemplates, TemplateInfo } from './project/templates';
import { compileShaders, createShaderFile, NewShaderArgs } from './shader/compileShaders';
import { buildStartProcessScript, launchElevated } from './util/exec';

const DEBUG_LAUNCH_NAME = 'DxLib: デバッグ実行 (Debug)';

/** 自動検証(test/)から使う窓口。利用者向けの機能ではない。 */
export interface DxLibTestApi {
	createProject: (args: CreateProjectArgs) => Promise<string>;
	collectEnvironment: typeof collectEnvironment;
	intelliSenseState: () => string;
	listTemplates: () => TemplateInfo[];
	showCreateForm: (location?: string) => Promise<void>;
	showNewShaderForm: () => Promise<void>;
	showSaveTemplateForm: () => Promise<void>;
	workloadInstallArgs: typeof workloadInstallArgs;
	buildStartProcessScript: typeof buildStartProcessScript;
}

export async function activate(context: vscode.ExtensionContext): Promise<DxLibTestApi> {
	const output = vscode.window.createOutputChannel('DxLib');
	const panel = new DxLibPanelProvider(context);
	const configProvider = new DxLibConfigurationProvider();
	context.subscriptions.push(
		output,
		vscode.window.registerWebviewViewProvider(DxLibPanelProvider.viewType, panel),
		vscode.tasks.registerTaskProvider(TASK_TYPE, new DxLibTaskProvider(context)),
		configProvider,
	);
	registerDxLibHover(context);
	registerHlslFormatter(context);

	// C/C++ 拡張の状態。DxLib プロジェクトを開いているときだけ「問い合わせが来たか」を見る。
	// C/C++ 拡張は、開いているフォルダの c_cpp_properties.json が自分たちを指していない限り
	// 設定プロバイダーに問い合わせてこない。DxLib プロジェクトを開いていないのに待ち続けると、
	// 待っても絶対に来ない問い合わせを 120 秒待って「応答しません」という誤報になる。
	let unresponsiveTimer: ReturnType<typeof setTimeout> | undefined;
	const armUnresponsiveTimer = (): void => {
		if (unresponsiveTimer) {
			clearTimeout(unresponsiveTimer);
			unresponsiveTimer = undefined;
		}
		const folder = currentFolder();
		if (intelliSenseState.get() !== 'preparing' || !folder || !isDxLibProject(folder)) {
			return; // DxLib プロジェクトを開いていなければ、問い合わせを待つ理由が無い
		}
		unresponsiveTimer = setTimeout(() => {
			if (intelliSenseState.get() === 'preparing') {
				intelliSenseState.set('unresponsive');
			}
		}, 120_000);
	};
	context.subscriptions.push({ dispose: () => unresponsiveTimer && clearTimeout(unresponsiveTimer) });

	if (cpptoolsInstalled()) {
		intelliSenseState.set('preparing');
		void configProvider.register(context);
		armUnresponsiveTimer();
	} else {
		void warnIfCpptoolsMissing();
	}
	context.subscriptions.push(
		intelliSenseState.onDidChange(() => void panel.refresh()),
		// フォルダを開き直した(例: DxLib プロジェクトを新規作成して開いた)ときに、
		// まだ「準備中」のままなら、そのタイミングでタイマーを張り直す。
		vscode.workspace.onDidChangeWorkspaceFolders(() => armUnresponsiveTimer()),
	);

	// 設定が変わったら再検出
	context.subscriptions.push(
		vscode.workspace.onDidChangeConfiguration((e) => {
			if (e.affectsConfiguration('dxlib')) {
				invalidateDxLibIndex();
				configProvider.invalidate();
				void panel.refresh();
			}
		}),
		vscode.workspace.onDidChangeWorkspaceFolders(() => void panel.refresh()),
	);

	// ステータスバーの「▶ 実行」
	const statusItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 50);
	statusItem.text = '$(play) DxLib 実行';
	statusItem.tooltip = 'ビルドして実行(デバッグなし)';
	statusItem.command = 'dxlib.run';
	context.subscriptions.push(statusItem);
	const updateStatusItem = (): void => {
		const folder = currentFolder();
		if (folder && isDxLibProject(folder)) {
			statusItem.show();
		} else {
			statusItem.hide();
		}
	};
	updateStatusItem();
	context.subscriptions.push(vscode.workspace.onDidChangeWorkspaceFolders(updateStatusItem));

	const register = (id: string, fn: (...args: unknown[]) => unknown): void => {
		context.subscriptions.push(vscode.commands.registerCommand(id, fn));
	};

	register('dxlib.refreshEnvironment', () => panel.refresh());

	register('dxlib.selectSdk', async () => {
		const picked = await vscode.window.showOpenDialog({
			canSelectFolders: true,
			canSelectFiles: false,
			canSelectMany: false,
			openLabel: 'このフォルダを SDK にする',
			title: '「プロジェクトに追加すべきファイル_VC用」フォルダを選んでください',
		});
		if (!picked?.[0]) {
			return;
		}
		// 間違ったフォルダでも保存する。パネルが ✗ になり、正しく設定されるまでプロジェクトを作成できない。
		const sdk = inspectSdk(picked[0].fsPath);
		await setGlobalConfig('sdkPath', picked[0].fsPath);
		if (!sdk?.version) {
			void vscode.window.showErrorMessage('DxLib.h が見つかりません。正しく設定されるまでプロジェクトは作成できません。「プロジェクトに追加すべきファイル_VC用」フォルダを選び直してください。');
		} else if (!sdk.ok) {
			void vscode.window.showWarningMessage(`DxLib ${sdk.version} を設定しましたが、足りないファイルがあります: ${sdk.missing.join(', ')}。正しく設定されるまでプロジェクトは作成できません。`);
		} else {
			void vscode.window.showInformationMessage(`DxLib ${sdk.version} を設定しました。`);
		}
	});

	register('dxlib.selectTemplatesDir', async () => {
		const picked = await vscode.window.showOpenDialog({ canSelectFolders: true, canSelectFiles: false, canSelectMany: false, openLabel: 'テンプレートフォルダにする' });
		if (picked?.[0]) {
			await setGlobalConfig('templatesPath', picked[0].fsPath);
		}
	});

	register('dxlib.createProject', async (arg?: unknown) => {
		// SDK が正しくない間は、フォームも開かず作成もしない(エクスプローラーの右クリックなど、どの入口からでも同じ)。
		const problem = sdkProblem();
		if (problem) {
			void vscode.window.showWarningMessage(`${problem}プロジェクトを作成できません。${SDK_FIX_HINT}`);
			return;
		}
		if (arg instanceof vscode.Uri) {
			await panel.showCreateForm(arg.fsPath);
			return;
		}
		const args = arg as CreateProjectArgs | undefined;
		if (!args || !args.name) {
			await panel.showCreateForm();
			return;
		}
		try {
			const dir = await createProject(context, args);
			await vscode.commands.executeCommand('vscode.openFolder', vscode.Uri.file(dir), { forceNewWindow: false });
		} catch (e) {
			void vscode.window.showErrorMessage(e instanceof Error ? e.message : String(e));
		}
	});

	register('dxlib.build', () => runBuild('debug'));

	register('dxlib.run', async () => {
		const folder = currentFolder();
		if (!folder) {
			void vscode.window.showWarningMessage('プロジェクトのフォルダが開かれていません。');
			return;
		}
		await vscode.debug.startDebugging(folder, DEBUG_LAUNCH_NAME, { noDebug: true });
	});

	register('dxlib.debug', async () => {
		const folder = currentFolder();
		if (!folder) {
			void vscode.window.showWarningMessage('プロジェクトのフォルダが開かれていません。');
			return;
		}
		await vscode.debug.startDebugging(folder, DEBUG_LAUNCH_NAME);
	});

	register('dxlib.compileShaders', () => compileShaders(output));

	register('dxlib.newShaderFile', async (arg?: unknown) => {
		const args = arg as NewShaderArgs | undefined;
		if (args) {
			const result = await createShaderFile(context.extensionPath, args);
			if (!result.ok) {
				void vscode.window.showErrorMessage(result.error ?? '不明なエラーです。');
				return;
			}
			await vscode.window.showTextDocument(vscode.Uri.file(result.file as string));
			await panel.refresh();
			return;
		}
		// 引数なし(コマンドパレット等からの呼び出し): フォームを開く前に、開く意味があるかだけ確認する。
		if (!currentFolder()) {
			void vscode.window.showWarningMessage('プロジェクトのフォルダが開かれていません。');
			return;
		}
		await panel.showNewShaderForm();
	});

	register('dxlib.saveAsTemplate', async (arg?: unknown) => {
		const args = arg as SaveTemplateArgs | undefined;
		if (args) {
			const result = await saveAsTemplateWork(args);
			if (!result.ok) {
				void vscode.window.showErrorMessage(result.error ?? '不明なエラーです。');
				return;
			}
			void vscode.window.showInformationMessage(`テンプレートとして保存しました(${result.count} ファイル): ${result.dest}`);
			await panel.refresh();
			return;
		}
		// 引数なし: フォームを開く前に、開いても使えない状態(フォルダ未指定)を先に案内する。
		if (!currentFolder()) {
			void vscode.window.showWarningMessage('プロジェクトのフォルダが開かれていません。');
			return;
		}
		const templatesPath = getConfig<string>('templatesPath', '');
		if (!templatesPath || !fs.existsSync(templatesPath)) {
			const pick = 'フォルダを指定';
			const choice = await vscode.window.showWarningMessage('テンプレートフォルダが設定されていません。先に指定してください。', pick);
			if (choice === pick) {
				await vscode.commands.executeCommand('dxlib.selectTemplatesDir');
			}
			return;
		}
		await panel.showSaveTemplateForm();
	});

	register('dxlib.createDefinition', async () => {
		if (!cpptoolsInstalled()) {
			await warnIfCpptoolsMissing();
			return;
		}
		await vscode.commands.executeCommand('C_Cpp.CreateDeclarationOrDefinition');
	});

	register('dxlib.openCpptools', () => vscode.commands.executeCommand('workbench.extensions.search', `@id:${CPPTOOLS_ID}`));

	register('dxlib.openVsDownload', () => vscode.env.openExternal(vscode.Uri.parse(getConfig<string>('visualStudioDownloadUrl', 'https://visualstudio.microsoft.com/ja/vs/community/'))));

	register('dxlib.openSetupGuide', async () => {
		const file = vscode.Uri.file(path.join(context.extensionPath, 'docs', 'setup-guide.md'));
		await vscode.commands.executeCommand('markdown.showPreview', file);
	});

	register('dxlib.addCppWorkload', async () => {
		const vs = await detectVisualStudio();
		const setup = installerSetupPath();
		if (vs.state === 'notFound' || !vs.installationPath || !setup) {
			await vscode.commands.executeCommand('dxlib.openVsDownload');
			return;
		}
		if (vs.state === 'ok') {
			void vscode.window.showInformationMessage('C++ ワークロードは既にインストールされています。');
			return;
		}
		const go = '追加する';
		const choice = await vscode.window.showInformationMessage(
			`${vs.displayName ?? 'Visual Studio'} に「C++ によるデスクトップ開発」を追加します。Visual Studio Installer が開くので、管理者権限の確認には「はい」を押してください。`,
			{ modal: true },
			go,
		);
		if (choice !== go) {
			return;
		}
		const r = await launchElevated(setup, workloadInstallArgs(vs.installationPath));
		if (r.code !== 0) {
			void vscode.window.showErrorMessage('Visual Studio Installer を起動できませんでした。手順ページを参考に手動で追加してください。');
		} else {
			void vscode.window.showInformationMessage('インストールが終わったら、DxLib パネルの「再チェック」を押してください。');
		}
	});

	// 初回起動時はウォークスルーを開く
	const shownKey = 'dxlib.walkthroughShown';
	if (!context.globalState.get<boolean>(shownKey)) {
		await context.globalState.update(shownKey, true);
		void vscode.commands.executeCommand('workbench.action.openWalkthrough', 'mahirocreative.dxlib-devenv#dxlib.gettingStarted', false);
	}

	// 開いているフォルダが DxLib プロジェクトなら、.vscode が無いときだけ案内(将来: 既存プロジェクトへの設定追加)
	const folder = currentFolder();
	if (folder && fs.existsSync(path.join(folder.uri.fsPath, 'src')) && !isDxLibProject(folder)) {
		output.appendLine(`[DxLib] ${folder.uri.fsPath} は DxLib プロジェクトとして認識されませんでした(.vscode/tasks.json に dxlib タスクがありません)。DxLib パネルからプロジェクトを作るか、設定を追加してください。`);
	}

	return {
		createProject: (args) => createProject(context, args),
		collectEnvironment,
		intelliSenseState: () => intelliSenseState.get(),
		listTemplates: () => listTemplates(context, getConfig<string>('templatesPath', '')),
		showCreateForm: (location) => panel.showCreateForm(location),
		showNewShaderForm: () => panel.showNewShaderForm(),
		showSaveTemplateForm: () => panel.showSaveTemplateForm(),
		workloadInstallArgs,
		buildStartProcessScript,
	};
}

export function deactivate(): void {
	// 何もしない(subscriptions で破棄される)
}
