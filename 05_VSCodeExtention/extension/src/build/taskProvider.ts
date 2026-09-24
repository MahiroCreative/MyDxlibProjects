import * as crypto from 'crypto';
import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { getConfig, isDxLibProject, projectExeName } from '../env/environment';
import { inspectSdk } from '../env/sdk';
import { detectVisualStudio } from '../env/vswhere';

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
 * vcvarsall や cl のコマンドラインは実行のたびに拡張機能が組み立てる。
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

		if (vs.state !== 'ok' || !vs.vcvarsall) {
			lines.push('echo [DxLib] Visual Studio の C++ ワークロードが見つかりません。DxLib パネルの環境欄を確認してください。', 'exit /b 1');
		} else if (!sdk || !sdk.ok) {
			lines.push('echo [DxLib] DxLib SDK が設定されていないか、ファイルが足りません。DxLib パネルで SDK フォルダを指定してください。', 'exit /b 1');
		} else {
			lines.push(...buildLines(folder, config, vs.vcvarsall, sdk.path, buildLogPath(script)));
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

/** cl の出力を残すログ(bat と同じ場所)。BuildDiagnostics がビルド後に読む。 */
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

function buildLines(folder: vscode.WorkspaceFolder, config: BuildConfig, vcvarsall: string, sdkPath: string, logPath: string): string[] {
	const exeName = projectExeName(folder);
	const outDirName = config === 'debug' ? 'Debug' : 'Release';
	const std = getConfig<string>('build.cppStandard', 'c++20', folder);
	const configFlags = config === 'debug' ? '/Od /MTd /Zi /D_DEBUG' : '/O2 /MT /DNDEBUG';
	const linkFlags = config === 'debug' ? '/DEBUG' : '';
	// 文字コード: プロジェクトのソースは BOM 付き UTF-8(BOM があるファイルは常に UTF-8 として読まれる)。
	// BOM の無いファイル(DxLib のヘッダーなど)は CP932 として読む。/source-charset:utf-8 にすると
	// DxLib.h の日本語コメントが 1 バイトごとに警告 C4828 になるので使わない。
	const common = `/nologo /EHsc /MP /W3 /wd4819 /std:${std} /source-charset:.932 /execution-charset:.932 /D_WINDOWS /DWIN32`;

	return [
		'setlocal enabledelayedexpansion',
		`set "PROJ=${folder.uri.fsPath}"`,
		`set "SDK=${sdkPath}"`,
		`set "OUT=%PROJ%\\build\\${outDirName}"`,
		`set "LOG=${logPath}"`,
		// 前回のログを残すと、今回 cl まで進まなかったときに古いエラーを表示してしまう
		'if exist "%LOG%" del "%LOG%"',
		'if not exist "%OUT%\\obj" mkdir "%OUT%\\obj"',
		`call "${vcvarsall}" x64 >nul 2>&1`,
		'if errorlevel 1 (',
		'  echo [DxLib] vcvarsall.bat の実行に失敗しました',
		'  exit /b 1',
		')',
		'set SRCS=',
		'for /r "%PROJ%\\src" %%f in (*.cpp) do set SRCS=!SRCS! "%%f"',
		'if not defined SRCS (',
		'  echo [DxLib] src フォルダに .cpp ファイルがありません',
		'  exit /b 1',
		')',
		'cd /d "%PROJ%"',
		`echo [DxLib] ${outDirName} ビルドを開始します`,
		// cl の出力はログに書いてから画面に出す(ログは BuildDiagnostics が読んで赤線にする)
		`cl ${common} ${configFlags} /I "%SDK%" /I "%PROJ%\\src" !SRCS! /Fo"%OUT%\\obj\\\\" /Fd"%OUT%\\${exeName}.pdb" /Fe"%OUT%\\${exeName}.exe" /link /SUBSYSTEM:WINDOWS /LIBPATH:"%SDK%" ${linkFlags} > "%LOG%" 2>&1`,
		'set CLERR=!errorlevel!',
		'type "%LOG%"',
		'if not "!CLERR!"=="0" (',
		'  echo [DxLib] ビルドに失敗しました',
		'  exit /b 1',
		')',
		`echo [DxLib] ビルド成功: %OUT%\\${exeName}.exe`,
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
