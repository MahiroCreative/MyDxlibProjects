import * as crypto from 'crypto';
import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { getConfig, isDxLibProject, projectExeName } from '../env/environment';
import { inspectSdk } from '../env/sdk';
import { detectVisualStudio } from '../env/vswhere';
import { ensureProjectFiles, projectFilePaths } from './vcxproj';
import { vsProjectOf } from './vsProject';

export const TASK_TYPE = 'dxlib';
export type BuildConfig = 'debug' | 'release';

interface DxLibTaskDefinition extends vscode.TaskDefinition {
	type: typeof TASK_TYPE;
	config: BuildConfig;
}

export function taskLabel(config: BuildConfig): string {
	return config === 'debug' ? 'Debug ビルド' : 'Release ビルド';
}

/**
 * タスク種別 `dxlib` を提供する。
 * プロジェクトの tasks.json には { "type": "dxlib", "config": "debug" } だけを書き、
 * MSBuild のコマンドラインは実行のたびに拡張機能が組み立てる(DESIGN.md 6 章)。
 */
export class DxLibTaskProvider implements vscode.TaskProvider {
	constructor(private readonly context: vscode.ExtensionContext) {}

	async provideTasks(): Promise<vscode.Task[]> {
		const tasks: vscode.Task[] = [];
		for (const folder of vscode.workspace.workspaceFolders ?? []) {
			// DxLib プロジェクト(.vscode/tasks.json に dxlib タスクがある)でなければ、
			// 「タスクの実行」の一覧にも出さない。無関係なフォルダでビルドが選べてしまうのを防ぐ。
			if (!isDxLibProject(folder)) {
				continue;
			}
			for (const config of ['debug', 'release'] as BuildConfig[]) {
				tasks.push(await this.createTask({ type: TASK_TYPE, config }, folder));
			}
		}
		return tasks;
	}

	async resolveTask(task: vscode.Task): Promise<vscode.Task | undefined> {
		const def = task.definition as DxLibTaskDefinition;
		if (def.type !== TASK_TYPE || (def.config !== 'debug' && def.config !== 'release')) {
			return undefined;
		}
		const folder =
			task.scope && typeof task.scope === 'object' ? (task.scope as vscode.WorkspaceFolder) : vscode.workspace.workspaceFolders?.[0];
		if (!folder) {
			return undefined;
		}
		return this.createTask(def, folder, task);
	}

	private async createTask(def: DxLibTaskDefinition, folder: vscode.WorkspaceFolder, base?: vscode.Task): Promise<vscode.Task> {
		const script = await this.writeBuildScript(folder, def.config);
		const execution = new vscode.ShellExecution({ value: script, quoting: vscode.ShellQuoting.Strong }, [], {
			executable: 'cmd.exe',
			shellArgs: ['/d', '/c'],
			cwd: folder.uri.fsPath,
		});
		// resolveTask では渡された definition オブジェクトをそのまま使う必要がある。
		// 問題マッチャーは使わない。ビルドエラーの赤線は BuildDiagnostics がログから付ける(直し始めたら消せるように)。
		const task = new vscode.Task(base?.definition ?? def, folder, taskLabel(def.config), TASK_TYPE, execution, []);
		task.group = vscode.TaskGroup.Build;
		task.presentationOptions = {
			reveal: vscode.TaskRevealKind.Always,
			panel: vscode.TaskPanelKind.Shared,
			clear: true,
			showReuseMessage: false,
		};
		return task;
	}

	/** ビルド用 bat を拡張機能の保存領域に書き、そのパスを返す。 */
	private async writeBuildScript(folder: vscode.WorkspaceFolder, config: BuildConfig): Promise<string> {
		const script = buildScriptPath(this.context, folder, config);
		fs.mkdirSync(path.dirname(script), { recursive: true });

		const vs = await detectVisualStudio();
		const sdk = inspectSdk(getConfig<string>('sdkPath', '') || undefined);
		const lines: string[] = ['@echo off', 'chcp 65001 >nul'];

		if (vs.state !== 'ok' || !vs.msbuild) {
			lines.push('echo [DxLib] Visual Studio の C++ ワークロード(MSBuild)が見つかりません。DxLib パネルの環境欄を確認してください。', 'exit /b 1');
		} else if (!sdk || !sdk.ok) {
			lines.push('echo [DxLib] DxLib SDK が設定されていないか、ファイルが足りません。DxLib パネルで SDK フォルダを指定してください。', 'exit /b 1');
		} else if (vsProjectOf(folder)) {
			// Visual Studio で作ったプロジェクトは、その .vcxproj をそのままビルドする(DESIGN.md 6.1 章)
			const vsp = vsProjectOf(folder) as { vcxproj: string; platform: string };
			lines.push(...buildLines(folder, def(config), vs.msbuild, vsp.vcxproj, vsp.platform, buildLogPath(script)));
		} else {
			// ビルドの前に .vcxproj・.sln・dxlib.props を今の設定に合わせる(無ければ作る)
			const name = projectExeName(folder);
			const ensured = ensureProjectFiles(folder.uri.fsPath, name, sdk.path, getConfig<string>('build.cppStandard', 'c++20', folder));
			if (ensured.foreignVcxproj) {
				lines.push(`echo [DxLib] ${path.basename(ensured.foreignVcxproj)} はこの拡張機能が作ったものではないので使えません。名前を変えるか移動してください。`, 'exit /b 1');
			} else {
				lines.push(...buildLines(folder, def(config), vs.msbuild, projectFilePaths(folder.uri.fsPath, name).vcxproj, 'x64', buildLogPath(script)));
			}
		}

		// chcp 65001 の後は UTF-8 として読まれるので BOM なし UTF-8 で書く。
		fs.writeFileSync(script, lines.join('\r\n') + '\r\n', 'utf8');
		return script;
	}
}

/** ビルド用 bat のパス。プロジェクトのパスと構成ごとに 1 つ(拡張機能の保存領域)。 */
export function buildScriptPath(context: vscode.ExtensionContext, folder: vscode.WorkspaceFolder, config: BuildConfig): string {
	const hash = crypto.createHash('md5').update(folder.uri.fsPath.toLowerCase()).digest('hex').slice(0, 8);
	return path.join(context.globalStorageUri.fsPath, 'build', `${projectExeName(folder)}_${hash}_${config}.bat`);
}

/** MSBuild の出力を残すログ(bat と同じ場所)。BuildDiagnostics がビルド後に読む。 */
export function buildLogPath(script: string): string {
	return script.replace(/\.bat$/i, '.log');
}

/** タスクからビルドログのパスを求める(dxlib タスクでなければ undefined)。 */
export function buildLogPathOfTask(context: vscode.ExtensionContext, task: vscode.Task): string | undefined {
	const def = task.definition as Partial<DxLibTaskDefinition>;
	if (def.type !== TASK_TYPE || (def.config !== 'debug' && def.config !== 'release')) {
		return undefined;
	}
	const folder = task.scope && typeof task.scope === 'object' ? (task.scope as vscode.WorkspaceFolder) : vscode.workspace.workspaceFolders?.[0];
	return folder ? buildLogPath(buildScriptPath(context, folder, def.config)) : undefined;
}

/** 構成の名前(MSBuild の Configuration)。 */
function def(config: BuildConfig): 'Debug' | 'Release' {
	return config === 'debug' ? 'Debug' : 'Release';
}

function buildLines(folder: vscode.WorkspaceFolder, cfg: 'Debug' | 'Release', msbuild: string, vcxproj: string, platform: string, logPath: string): string[] {
	return [
		'setlocal',
		`cd /d "${folder.uri.fsPath}"`,
		`set "LOG=${logPath}"`,
		// 前回のログを残すと、今回 MSBuild まで進まなかったときに古いエラーを表示してしまう
		'if exist "%LOG%" del "%LOG%"',
		`echo [DxLib] ${cfg} ビルドを開始します`,
		// 変更したファイルだけをコンパイルし直す。ログは BuildDiagnostics が読んで赤線にする(画面にも同じ内容が出る)
		`"${msbuild}" "${vcxproj}" -p:Configuration=${cfg} -p:Platform=${platform} -nologo -v:minimal -m "-flp:logfile=%LOG%;verbosity=minimal;encoding=utf-8"`,
		'if errorlevel 1 (',
		'  echo [DxLib] ビルドに失敗しました',
		'  exit /b 1',
		')',
		// exe の場所は MSBuild の出力(「<名前>.vcxproj -> <exe>」)に出ている
		`echo [DxLib] ${cfg} ビルドに成功しました`,
		'endlocal',
		'exit /b 0',
	];
}

/** 指定構成の dxlib タスクを探して実行する。 */
export async function runBuild(config: BuildConfig): Promise<vscode.TaskExecution | undefined> {
	const folder = vscode.workspace.workspaceFolders?.[0];
	if (!folder) {
		void vscode.window.showWarningMessage('プロジェクトのフォルダが開かれていません。');
		return undefined;
	}
	const tasks = await vscode.tasks.fetchTasks({ type: TASK_TYPE });
	const task = tasks.find((t) => (t.definition as DxLibTaskDefinition).config === config && t.scope === folder) ?? tasks.find((t) => (t.definition as DxLibTaskDefinition).config === config);
	if (!task) {
		void vscode.window.showErrorMessage('DxLib のビルドタスクが見つかりません。');
		return undefined;
	}
	return vscode.tasks.executeTask(task);
}
