// 実機検証: インストール済みの VSCode を別プロファイルで起動し、拡張機能を実際に動かす。
//   段階 1: 環境検出とプロジェクト作成(フォルダを開かずに)
//   段階 2: 作ったプロジェクトを開いて、ビルド・ホバー・IntelliSense・シェーダー・デバッグ実行
//   段階 3〜5: 見た目・シェーダー雛形の描画・install.bat
//   段階 6: C/C++ 拡張が DxLib 拡張より後から入る順番
//   段階 7: 制限モード(信頼されていないフォルダ)
//   段階 8: C/C++ Extension Pack(CMake Tools を含む)を入れた状態
// 使い方: node test/runTest.js <作業フォルダ> <SDK フォルダ> [--phase6-only | --phase7-only | --phase8-only]
//   --phaseN-only は、全段階を 1 回通した作業フォルダでその段階だけを走らせる
const fs = require('fs');
const path = require('path');
const { execFileSync, spawn } = require('child_process');
const { runTests } = require('@vscode/test-electron');

// Claude Code などの VSCode 拡張から起動すると引き継がれ、Code.exe が Node として動いてしまう
delete process.env.ELECTRON_RUN_AS_NODE;

const CODE_DIR = path.join(process.env.LOCALAPPDATA, 'Programs', 'Microsoft VS Code');
const CODE_EXE = path.join(CODE_DIR, 'Code.exe');
const CODE_CLI = path.join(CODE_DIR, 'bin', 'code.cmd');

async function main() {
	const work = path.resolve(process.argv[2]);
	const sdk = process.argv[3];
	const only6 = process.argv[4] === '--phase6-only';
	const only7 = process.argv[4] === '--phase7-only';
	const only8 = process.argv[4] === '--phase8-only';
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
	if (only6 || only7 || only8) {
		if (!fs.existsSync(project)) {
			throw new Error('段階 6〜8 だけを走らせるには、先に全段階を 1 回通して TestGame を作っておく');
		}
		if (only6) {
			await phase6(common, userData, project, work);
		} else if (only7) {
			await phase7(common, extDir, project, work, sdk);
		} else {
			await phase8(common, extDir, userData, project, work);
		}
		console.log('[runTest] 完了');
		return;
	}
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

	await phase6(common, userData, project, work);
	await phase7(common, extDir, project, work, sdk);
	await phase8(common, extDir, userData, project, work);

	console.log('[runTest] 完了');
}

// 段階 8: C/C++ Extension Pack(CMake Tools を含む)を入れた状態でも動くか。
// 生徒が VSCode のおすすめに従って入れてしまった場合を想定する。
async function phase8(common, mainExtDir, userData, project, work) {
	const extDir = path.join(work, 'extensions-pack');
	if (!fs.existsSync(path.join(extDir, 'extensions.json')) || !fs.readdirSync(extDir).some((n) => n.startsWith('ms-vscode.cpptools-extension-pack'))) {
		console.log('[runTest] 段階 8 の準備: C/C++ Extension Pack をインストール');
		fs.rmSync(extDir, { recursive: true, force: true });
		fs.cpSync(mainExtDir, extDir, { recursive: true });
		execFileSync('cmd.exe', ['/c', CODE_CLI, '--extensions-dir', extDir, '--user-data-dir', userData, '--install-extension', 'ms-vscode.cpptools-extension-pack'], { stdio: 'inherit' });
	}
	// 以前の版で作ったプロジェクトの移行も、ここで確かめる(DESIGN.md 6 章)。
	// tasks.json をわざと以前の形(problemMatcher: ["$msCompile"])に戻してから開く。
	const tasksFile = path.join(project, '.vscode', 'tasks.json');
	const tasks = JSON.parse(fs.readFileSync(tasksFile, 'utf8'));
	for (const t of tasks.tasks) {
		t.problemMatcher = ['$msCompile'];
	}
	fs.writeFileSync(tasksFile, JSON.stringify(tasks, null, '\t') + '\n');
	console.log('[runTest] 段階 8(C/C++ Extension Pack あり)');
	await runTests({
		...common,
		extensionTestsPath: path.join(__dirname, 'suite', 'phase8.js'),
		launchArgs: ['--user-data-dir', userData, '--extensions-dir', extDir, '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust', project],
	});
}

// 段階 7: 制限モード(信頼されていないフォルダ)で開く(DESIGN.md 17.4)。
// 他の段階は信頼の確認を切っているので、信頼の確認を有効にした別の user-data で起動する。
// 開発中の拡張(extensionDevelopmentPath)は信頼の制限を受けないので、DxLib 拡張は VSIX を作って
// 段階 7 専用の拡張フォルダにインストールし、生徒の PC と同じ「インストールされた拡張」として動かす。
// 検証コードは中身の無い検証用の拡張(trust-harness)として読み込む。
async function phase7(common, mainExtDir, project, work, sdk) {
	const extDir = path.join(work, 'extensions-trust');
	if (!fs.existsSync(extDir)) {
		// C/C++ 拡張などの依存を毎回ダウンロードしないよう、段階 1〜6 の拡張フォルダを元にする
		fs.cpSync(mainExtDir, extDir, { recursive: true });
	}
	const vsix = path.join(work, 'dxlib-devenv-test.vsix');
	console.log('[runTest] 段階 7 の準備: VSIX を作ってインストール');
	execFileSync('cmd.exe', ['/c', 'npx', 'vsce', 'package', '--out', vsix], { cwd: common.extensionDevelopmentPath, stdio: 'inherit' });
	execFileSync('cmd.exe', ['/c', CODE_CLI, '--extensions-dir', extDir, '--user-data-dir', path.join(work, 'user-data'), '--install-extension', vsix, '--force'], { stdio: 'inherit' });
	const harness = path.join(work, 'trust-harness');
	fs.mkdirSync(harness, { recursive: true });
	fs.writeFileSync(
		path.join(harness, 'package.json'),
		JSON.stringify({ name: 'dxlib-trust-harness', publisher: 'dxlibtest', version: '0.0.1', engines: { vscode: '^1.90.0' }, main: './noop.js', capabilities: { untrustedWorkspaces: { supported: true } } }, null, 2),
	);
	fs.writeFileSync(path.join(harness, 'noop.js'), 'exports.activate = () => {};\n');

	const userData = path.join(work, 'user-data-trust');
	fs.rmSync(userData, { recursive: true, force: true }); // 以前に信頼したフォルダの記録を残さない
	fs.mkdirSync(path.join(userData, 'User'), { recursive: true });
	fs.writeFileSync(
		path.join(userData, 'User', 'settings.json'),
		JSON.stringify(
			{
				'dxlib.sdkPath': sdk,
				'security.workspace.trust.enabled': true,
				// 起動時の「信頼しますか」ダイアログは出さない(閉じた・いいえ を選んだのと同じ、制限モードで開く)
				'security.workspace.trust.startupPrompt': 'never',
				'workbench.startupEditor': 'none',
				'extensions.autoUpdate': false,
			},
			null,
			2,
		),
	);
	console.log('[runTest] 段階 7(制限モード)');
	// @vscode/test-electron の runTests は必ず --disable-workspace-trust を付けるので、制限モードにならない。
	// ここだけは同じ引数から --disable-workspace-trust を除いて、VSCode を直接起動する。
	const args = [
		'--no-sandbox',
		'--disable-gpu-sandbox',
		'--disable-updates',
		'--skip-welcome',
		'--skip-release-notes',
		'--no-cached-data',
		`--extensionTestsPath=${path.join(__dirname, 'suite', 'phase7.js')}`,
		`--extensionDevelopmentPath=${harness}`,
		'--user-data-dir',
		userData,
		'--extensions-dir',
		extDir,
		project,
	];
	const env = { ...process.env, ...common.extensionTestsEnv, DXLIB_USER_DATA_DIR: userData };
	const code = await new Promise((resolve) => {
		const p = spawn(common.vscodeExecutablePath, args, { env });
		p.stdout.on('data', (d) => process.stdout.write(d));
		p.stderr.on('data', (d) => process.stderr.write(d));
		p.on('close', (c) => resolve(c));
	});
	if (code !== 0) {
		throw new Error(`段階 7 が失敗しました(終了コード ${code})`);
	}
}

// 段階 6: C/C++ 拡張の無い拡張フォルダで起動し、動いている間に C/C++ 拡張を入れる(DESIGN.md 17.3)。
async function phase6(common, userData, project, work) {
	// 段階 1 の前に入れた C/C++ 拡張の VSIX が、VSCode のキャッシュに残っている。
	// これを使うと、install.bat と同じくダウンロード無しの速さで入る。
	const cacheDir = path.join(userData, 'CachedExtensionVSIXs');
	const cached = fs.existsSync(cacheDir) ? fs.readdirSync(cacheDir).find((n) => n.startsWith('ms-vscode.cpptools-') && !n.endsWith('.sigzip')) : undefined;
	if (!cached) {
		throw new Error(`C/C++ 拡張の VSIX がキャッシュにありません: ${cacheDir}`);
	}
	const vsix = path.join(work, 'cpptools.vsix'); // code --install-extension は拡張子 .vsix を要る
	fs.copyFileSync(path.join(cacheDir, cached), vsix);

	for (const mode of ['late', 'soon']) {
		const lateExtDir = path.join(work, `extensions-${mode}`);
		fs.rmSync(lateExtDir, { recursive: true, force: true });
		fs.mkdirSync(lateExtDir, { recursive: true });
		console.log(`[runTest] 段階 6-${mode}(C/C++ 拡張が後から入る)`);
		await runTests({
			...common,
			extensionTestsEnv: {
				...common.extensionTestsEnv,
				DXLIB_TEST_LATE_MODE: mode,
				DXLIB_LATE_EXT_DIR: lateExtDir,
				DXLIB_CPPTOOLS_VSIX: vsix,
				DXLIB_CODE_CLI: CODE_CLI,
			},
			extensionTestsPath: path.join(__dirname, 'suite', 'phase6.js'),
			launchArgs: ['--user-data-dir', userData, '--extensions-dir', lateExtDir, '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust', project],
		});
	}
}

main().catch((e) => {
	console.error('[runTest] 失敗:', e && e.message ? e.message : e);
	process.exit(1);
});
