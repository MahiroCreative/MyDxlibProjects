// 実機検証: インストール済みの VSCode を別プロファイルで起動し、拡張機能を実際に動かす。
//   段階 1: 環境検出とプロジェクト作成(フォルダを開かずに)
//   段階 2: 作ったプロジェクトを開いて、ビルド・ホバー・IntelliSense・シェーダー・デバッグ実行
// 使い方: node test/runTest.js <作業フォルダ> <SDK フォルダ>
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');
const { runTests } = require('@vscode/test-electron');

// Claude Code などの VSCode 拡張から起動すると引き継がれ、Code.exe が Node として動いてしまう
delete process.env.ELECTRON_RUN_AS_NODE;

const CODE_DIR = path.join(process.env.LOCALAPPDATA, 'Programs', 'Microsoft VS Code');
const CODE_EXE = path.join(CODE_DIR, 'Code.exe');
const CODE_CLI = path.join(CODE_DIR, 'bin', 'code.cmd');

async function main() {
	const work = path.resolve(process.argv[2]);
	const sdk = process.argv[3];
	const extDev = path.resolve(__dirname, '..');
	const userData = path.join(work, 'user-data');
	const extDir = path.join(work, 'extensions');
	const projects = path.join(work, 'projects');
	fs.mkdirSync(path.join(userData, 'User'), { recursive: true });
	fs.mkdirSync(projects, { recursive: true });

	// 利用者設定: SDK パス。ウォークスルーなどの初回表示は検証の邪魔なので切る。
	fs.writeFileSync(
		path.join(userData, 'User', 'settings.json'),
		JSON.stringify({ 'dxlib.sdkPath': sdk, 'security.workspace.trust.enabled': false, 'workbench.startupEditor': 'none', 'extensions.autoUpdate': false }, null, 2),
	);

	// C/C++ 拡張(デバッグと IntelliSense に必要)を検証用の拡張フォルダに入れる
	if (!fs.existsSync(extDir) || !fs.readdirSync(extDir).some((n) => n.startsWith('ms-vscode.cpptools'))) {
		console.log('[runTest] C/C++ 拡張をインストールします');
		execFileSync('cmd.exe', ['/c', CODE_CLI, '--extensions-dir', extDir, '--user-data-dir', userData, '--install-extension', 'ms-vscode.cpptools'], { stdio: 'inherit' });
	}

	const common = {
		vscodeExecutablePath: CODE_EXE,
		extensionDevelopmentPath: extDev,
		extensionTestsEnv: { DXLIB_TEST_WORK: work, DXLIB_TEST_PROJECTS: projects, DXLIB_USER_DATA_DIR: userData },
	};
	const baseArgs = ['--user-data-dir', userData, '--extensions-dir', extDir, '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust'];

	const project = path.join(projects, 'TestGame');
	if (fs.existsSync(project)) {
		fs.rmSync(project, { recursive: true, force: true });
	}

	console.log('[runTest] 段階 1');
	await runTests({ ...common, extensionTestsPath: path.join(__dirname, 'suite', 'phase1.js'), launchArgs: [...baseArgs] });

	console.log('[runTest] 段階 2');
	await runTests({ ...common, extensionTestsPath: path.join(__dirname, 'suite', 'phase2.js'), launchArgs: [...baseArgs, project] });

	console.log('[runTest] 段階 3(見た目のスクリーンショット)');
	await runTests({ ...common, extensionTestsPath: path.join(__dirname, 'suite', 'phase3.js'), launchArgs: [...baseArgs, project] });

	console.log('[runTest] 段階 4(シェーダー雛形を DxLib で描画)');
	execFileSync(process.execPath, [path.join(__dirname, 'shader-runtime', 'run.js'), work, sdk], { stdio: 'inherit' });

	console.log('[runTest] 段階 5(配布用 install.bat)');
	execFileSync(process.execPath, [path.join(__dirname, 'install-bat', 'run.js'), work], { stdio: 'inherit' });

	console.log('[runTest] 完了');
}

main().catch((e) => {
	console.error('[runTest] 失敗:', e && e.message ? e.message : e);
	process.exit(1);
});
