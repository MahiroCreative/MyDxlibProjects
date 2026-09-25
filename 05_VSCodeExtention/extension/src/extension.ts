import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { BuildDiagnostics } from './build/buildDiagnostics';
import { buildLogPathOfTask, DxLibTaskProvider, runBuild, TASK_TYPE } from './build/taskProvider';
import { migrateProject } from './project/migrate';
import { CPPTOOLS_ID, cpptoolsInstalled, intelliSenseState, warnIfCpptoolsMissing } from './env/cpptools';
import { registerHlslFormatter } from './format/hlslFormatter';
import { collectEnvironment, currentFolder, getConfig, isDxLibProject, SDK_FIX_HINT, sdkProblem, setGlobalConfig } from './env/environment';
import { inspectSdk } from './env/sdk';
import { detectVisualStudio, installerSetupPath, workloadInstallArgs } from './env/vswhere';
import { invalidateDxLibIndex, registerDxLibHover } from './hover/dxlibHover';
import { DxLibConfigurationProvider } from './intellisense/configProvider';
import { DxLibPanelProvider } from './panel/panelView';
import { DxLibProjectViewProvider } from './panel/projectView';
import { createProject, CreateProjectArgs, defaultCreateLocation } from './project/createProject';
import { saveAsTemplateWork, SaveTemplateArgs } from './project/saveAsTemplate';
import { listTemplates, TemplateInfo } from './project/templates';
import { compileShaders, createShaderFile, NewShaderArgs, shaderSourceDir } from './shader/compileShaders';
import { buildStartProcessScript, launchElevated } from './util/exec';

const DEBUG_LAUNCH_NAME = 'DxLib: デバッグ実行 (Debug)';

/** 自動検証(test/)から使う窓口。利用者向けの機能ではない。 */
export interface DxLibTestApi {
	/** 制限モードでなく、ビルドなどの機能が起動しているか。 */
	trustedFeaturesActive: () => boolean;
	createProject: (args: CreateProjectArgs) => Promise<string>;
	collectEnvironment: typeof collectEnvironment;
	intelliSenseState: () => string;
	lastIntelliSenseQuery: () => { at: number; includePath: string[] } | undefined;
	listTemplates: () => TemplateInfo[];
	/** 作成フォームの「作成先」の初期値(前回の作成先)。 */
	defaultCreateLocation: () => string;
	showCreateForm: (location?: string) => Promise<void>;
	showNewShaderForm: () => Promise<void>;
	showSaveTemplateForm: () => Promise<void>;
	workloadInstallArgs: typeof workloadInstallArgs;
	buildStartProcessScript: typeof buildStartProcessScript;
}

export async function activate(context: vscode.ExtensionContext): Promise<DxLibTestApi> {
	const output = vscode.window.createOutputChannel('DxLib');
	const panel = new DxLibPanelProvider(context);
	context.subscriptions.push(
		output,
		vscode.window.registerWebviewViewProvider(DxLibPanelProvider.viewType, panel),
		vscode.commands.registerCommand('dxlib.manageTrust', () => vscode.commands.executeCommand('workbench.trust.manage')),
	);
	if (vscode.workspace.isTrusted) {
		return activateTrusted(context, panel, output);
	}

	// 制限モード(信頼されていないフォルダ)。パネルには信頼の案内だけを出し、
	// フォルダの中身に基づいてプロセスを動かす機能(ビルド・補完・シェーダー・整形など)は起動しない。
	// 信頼されたら、その場で残りを起動する(開き直し不要)。
	let trustedApi: DxLibTestApi | undefined;
	context.subscriptions.push(
		vscode.workspace.onDidGrantWorkspaceTrust(async () => {
			trustedApi = await activateTrusted(context, panel, output);
			await panel.refresh();
		}),
	);
	const notYet = (): never => {
		throw new Error('制限モードのため、まだ起動していません');
	};
	return {
		trustedFeaturesActive: () => !!trustedApi,
		createProject: (args) => (trustedApi ?? notYet()).createProject(args),
		collectEnvironment: () => (trustedApi ?? notYet()).collectEnvironment(),
		intelliSenseState: () => (trustedApi ?? notYet()).intelliSenseState(),
		lastIntelliSenseQuery: () => (trustedApi ?? notYet()).lastIntelliSenseQuery(),
		listTemplates: () => (trustedApi ?? notYet()).listTemplates(),
		defaultCreateLocation: () => (trustedApi ?? notYet()).defaultCreateLocation(),
		showCreateForm: (location) => (trustedApi ?? notYet()).showCreateForm(location),
		showNewShaderForm: () => (trustedApi ?? notYet()).showNewShaderForm(),
		showSaveTemplateForm: () => (trustedApi ?? notYet()).showSaveTemplateForm(),
		workloadInstallArgs,
		buildStartProcessScript,
	};
}

/** 信頼されたフォルダ(または空の窓)で動かす機能をすべて起動する。 */
async function activateTrusted(context: vscode.ExtensionContext, panel: DxLibPanelProvider, output: vscode.OutputChannel): Promise<DxLibTestApi> {
	const configProvider = new DxLibConfigurationProvider(context.globalStorageUri.fsPath);
	context.subscriptions.push(
		vscode.tasks.registerTaskProvider(TASK_TYPE, new DxLibTaskProvider(context)),
		configProvider,
		// ビルドエラーの赤線(直し始めたら消す)
		new BuildDiagnostics((task) => buildLogPathOfTask(context, task)),
	);

	// エクスプローラーの「DxLib」欄(DESIGN.md 3.1 章)。プロジェクトの操作とそのフォームはここ。
	const projectView = new DxLibProjectViewProvider(context);
	context.subscriptions.push(vscode.window.registerWebviewViewProvider(DxLibProjectViewProvider.viewType, projectView));

	// 欄と右クリックのメニューの表示条件。dxlib.isProject: DxLib プロジェクトを開いているか。
	// dxlib.shaderFolders: シェーダーのフォルダのパス(when の「resourcePath in ...」で使う。ドライブ文字の大小の両方)。
	const updateContext = (): void => {
		const folder = currentFolder();
		const isProject = !!folder && isDxLibProject(folder);
		void vscode.commands.executeCommand('setContext', 'dxlib.isProject', isProject);
		const folders: Record<string, true> = {};
		if (folder && isProject) {
			const dir = shaderSourceDir(folder);
			folders[dir] = true;
			folders[dir.charAt(0).toLowerCase() + dir.slice(1)] = true;
			folders[dir.charAt(0).toUpperCase() + dir.slice(1)] = true;
		}
		void vscode.commands.executeCommand('setContext', 'dxlib.shaderFolders', folders);
	};
	updateContext();
	context.subscriptions.push(
		vscode.workspace.onDidChangeWorkspaceFolders(updateContext),
		vscode.workspace.onDidChangeConfiguration((e) => e.affectsConfiguration('dxlib.shader') && updateContext()),
	);

	// 以前の版で作ったプロジェクトの設定を今の形に直す
	const migrate = (): void => {
		const folder = currentFolder();
		if (folder && isDxLibProject(folder)) {
			for (const message of migrateProject(folder)) {
				output.appendLine(`[DxLib] ${message}`);
				void vscode.window.showInformationMessage(message);
			}
		}
	};
	migrate();
	context.subscriptions.push(vscode.workspace.onDidChangeWorkspaceFolders(migrate));
	registerDxLibHover(context);
	registerHlslFormatter(context);

	// C/C++ 拡張の状態。問い合わせが来るはずの状況でだけ「来たか」を見る。
	// C/C++ 拡張は、開いているフォルダの c_cpp_properties.json が自分たちを指していて、
	// かつ C/C++ のファイルがエディタで開かれるまで、設定プロバイダーに問い合わせてこない。
	// それ以外の状況で待つと、待っても絶対に来ない問い合わせを 120 秒待って「応答しません」という誤報になる。
	const isCppDocument = (d: vscode.TextDocument): boolean =>
		d.uri.scheme === 'file' && /\.(c|cc|cpp|cxx|h|hpp|hxx|inl)$/i.test(d.uri.fsPath);
	let unresponsiveTimer: ReturnType<typeof setTimeout> | undefined;
	const stopUnresponsiveTimer = (): void => {
		if (unresponsiveTimer) {
			clearTimeout(unresponsiveTimer);
			unresponsiveTimer = undefined;
		}
	};
	const armUnresponsiveTimer = (): void => {
		const folder = currentFolder();
		const shouldWait =
			intelliSenseState.get() === 'preparing' &&
			!!folder &&
			isDxLibProject(folder) &&
			vscode.workspace.textDocuments.some(isCppDocument);
		if (!shouldWait) {
			stopUnresponsiveTimer();
			return;
		}
		if (unresponsiveTimer) {
			return; // すでに待っている。ファイルを開くたびに張り直すと、いつまでも判定しない
		}
		unresponsiveTimer = setTimeout(() => {
			unresponsiveTimer = undefined;
			if (intelliSenseState.get() === 'preparing') {
				intelliSenseState.set('unresponsive');
			}
		}, 120_000);
	};
	context.subscriptions.push({ dispose: stopUnresponsiveTimer });

	// C/C++ 拡張が使えるようになったら「準備中」にして設定プロバイダーを登録する。
	// install.bat(code --install-extension)は DxLib 拡張を先に入れ、C/C++ 拡張は数秒遅れて入る。
	// VSCode を開いたままだと DxLib 拡張が先に起動するので、起動時の 1 回だけの判定では足りない。
	let providerRegistered = false;
	const startCpptools = (): void => {
		if (intelliSenseState.get() !== 'missing') {
			return;
		}
		intelliSenseState.set('preparing');
		if (!providerRegistered) {
			providerRegistered = true;
			void configProvider.register(context);
		}
		armUnresponsiveTimer();
	};
	if (cpptoolsInstalled()) {
		startCpptools();
	} else {
		// すぐ警告すると、後から入る途中の C/C++ 拡張を「未インストール」と誤報する。少し待ってから判断する。
		const warnTimer = setTimeout(() => {
			if (!cpptoolsInstalled()) {
				void warnIfCpptoolsMissing();
			}
		}, 15_000);
		context.subscriptions.push({ dispose: () => clearTimeout(warnTimer) });
	}
	context.subscriptions.push(
		vscode.extensions.onDidChange(() => {
			if (cpptoolsInstalled()) {
				startCpptools();
			} else {
				intelliSenseState.set('missing');
			}
		}),
		intelliSenseState.onDidChange(() => void panel.refresh()),
		// フォルダを開き直した・C/C++ のファイルを開いた/閉じたときに、待つ条件を見直す。
		vscode.workspace.onDidChangeWorkspaceFolders(() => armUnresponsiveTimer()),
		vscode.workspace.onDidOpenTextDocument((d) => isCppDocument(d) && armUnresponsiveTimer()),
		vscode.workspace.onDidCloseTextDocument((d) => isCppDocument(d) && armUnresponsiveTimer()),
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
			// フォルダを開いている窓なら新しい窓で開き、開いていた作業を閉じない。空の窓ならその窓で開く。
			const forceNewWindow = (vscode.workspace.workspaceFolders?.length ?? 0) > 0;
			await vscode.commands.executeCommand('vscode.openFolder', vscode.Uri.file(dir), { forceNewWindow });
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

	// エクスプローラーの右クリック「このシェーダーをコンパイル」。複数選択のときは 2 つ目の引数に全部が来る。
	register('dxlib.compileShaderFile', async (arg?: unknown, all?: unknown) => {
		const uris = (Array.isArray(all) && all.length > 0 ? all : [arg]).filter((u): u is vscode.Uri => u instanceof vscode.Uri);
		if (uris.length === 0) {
			void vscode.window.showWarningMessage('エクスプローラーでシェーダーのファイルを右クリックして使ってください。');
			return;
		}
		await compileShaders(output, uris.map((u) => u.fsPath));
	});

	register('dxlib.showProjectView', () => vscode.commands.executeCommand(`${DxLibProjectViewProvider.viewType}.focus`));

	register('dxlib.newShaderFile', async (arg?: unknown) => {
		// エクスプローラーの右クリックからはフォルダの Uri が来る。そのときはフォームを開く。
		const args = arg instanceof vscode.Uri ? undefined : (arg as NewShaderArgs | undefined);
		if (args) {
			const result = await createShaderFile(context.extensionPath, args);
			if (!result.ok) {
				void vscode.window.showErrorMessage(result.error ?? '不明なエラーです。');
				return;
			}
			await vscode.window.showTextDocument(vscode.Uri.file(result.file as string));
			return;
		}
		// 引数なし(コマンドパレット等からの呼び出し): フォームを開く前に、開く意味があるかだけ確認する。
		if (!currentFolder()) {
			void vscode.window.showWarningMessage('プロジェクトのフォルダが開かれていません。');
			return;
		}
		await projectView.showNewShaderForm();
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
			await panel.refresh(); // パネルの作成フォームのテンプレート一覧を更新
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
		await projectView.showSaveTemplateForm();
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
		trustedFeaturesActive: () => true,
		createProject: (args) => createProject(context, args),
		collectEnvironment,
		intelliSenseState: () => intelliSenseState.get(),
		lastIntelliSenseQuery: () => configProvider.lastProvided,
		listTemplates: () => listTemplates(context, getConfig<string>('templatesPath', '')),
		defaultCreateLocation: () => defaultCreateLocation(context),
		showCreateForm: (location) => panel.showCreateForm(location),
		showNewShaderForm: () => projectView.showNewShaderForm(),
		showSaveTemplateForm: () => projectView.showSaveTemplateForm(),
		workloadInstallArgs,
		buildStartProcessScript,
	};
}

export function deactivate(): void {
	// 何もしない(subscriptions で破棄される)
}
