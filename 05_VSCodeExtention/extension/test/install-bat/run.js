// 段階 5: 配布用 install.bat の検証。
//   偽の code.cmd(受け取った引数を記録するだけ)で、code の探し方・エラー案内・日本語と空白のパスを確かめ、
//   最後に本物の VSCode(検証用プロファイル)へ実際にインストールする。
// 使い方: node test/install-bat/run.js <作業フォルダ>
const fs = require('fs');
const os = require('os');
const path = require('path');
const { spawnSync } = require('child_process');

const work = path.resolve(process.argv[2], 'install-bat');
const extRoot = path.resolve(__dirname, '..', '..');
const batSrc = path.join(extRoot, 'release-files', 'install.bat');
const realVsix = fs.readdirSync(extRoot).filter((f) => /^dxlib-devenv-.*\.vsix$/.test(f)).sort().pop();

let ng = 0;
function check(name, ok, detail) {
	if (!ok) {
		ng++;
	}
	console.log(`[phase5] ${ok ? 'OK' : 'NG'}  ${name}${detail ? '  : ' + detail : ''}`);
}

fs.rmSync(work, { recursive: true, force: true });
fs.mkdirSync(work, { recursive: true });

// 配布フォルダに見立てた、空白と日本語を含むフォルダ(生徒の「ダウンロード」などを想定)
const dist = path.join(work, '配布 フォルダ');
fs.mkdirSync(dist);
fs.copyFileSync(batSrc, path.join(dist, 'install.bat'));

/** 偽の code.cmd を作る。呼ばれたら、引数をログに書く。 */
function makeStub(dir, log) {
	fs.mkdirSync(dir, { recursive: true });
	fs.writeFileSync(path.join(dir, 'code.cmd'), `@echo off\r\necho %*> "${log}"\r\nexit /b 0\r\n`);
}

/** install.bat を実行する。ダブルクリックと同じく cmd.exe が bat を開く。 */
function runBat(env, cwd = dist) {
	const merged = { ...env, DXLIB_INSTALL_NOPAUSE: '1' };
	delete merged.ELECTRON_RUN_AS_NODE;
	// cmd /s /c は外側の引用符を 1 組はがす。空白を含むパスを守るため、二重に囲む。
	const r = spawnSync('cmd.exe', ['/d', '/s', '/c', `""${path.join(cwd, 'install.bat')}""`], { env: merged, windowsVerbatimArguments: true, cwd, timeout: 600000 });
	return { code: r.status, out: (r.stdout || Buffer.alloc(0)).toString('utf8'), err: (r.stderr || Buffer.alloc(0)).toString('utf8') };
}

const sys32 = path.join(process.env.SystemRoot, 'System32');
const emptyDir = (n) => {
	const d = path.join(work, n);
	fs.mkdirSync(d, { recursive: true });
	return d;
};
/** code が入っていそうな場所を全部空の場所にした環境。 */
const baseEnv = (over = {}) => ({
	SystemRoot: process.env.SystemRoot,
	PATH: sys32,
	LOCALAPPDATA: emptyDir('empty-local'),
	ProgramFiles: emptyDir('empty-pf'),
	'ProgramFiles(x86)': emptyDir('empty-pf86'),
	...over,
});

// ダミーの VSIX(偽の code は中身を見ない)
fs.writeFileSync(path.join(dist, 'dxlib-devenv-9.9.9.vsix'), 'dummy');
const dummyVsix = path.join(dist, 'dxlib-devenv-9.9.9.vsix');
const expectedArgs = (log) => {
	const t = fs.existsSync(log) ? fs.readFileSync(log, 'utf8').trim() : '';
	return { text: t, ok: t === `--install-extension "${dummyVsix}" --force` };
};

// 1. PATH に code がある
{
	const log = path.join(work, 'log1.txt');
	const stub = path.join(work, 'stub-path');
	makeStub(stub, log);
	const r = runBat(baseEnv({ PATH: `${stub};${sys32}` }));
	const a = expectedArgs(log);
	check('1. PATH の code を使う(空白・日本語のフォルダから実行)', r.code === 0 && a.ok && r.out.includes('インストールが完了しました'), `exit=${r.code} args=${a.text}`);
}

// 2. PATH に無く、ユーザー版の標準の場所にある
{
	const log = path.join(work, 'log2.txt');
	const local = path.join(work, 'local2');
	makeStub(path.join(local, 'Programs', 'Microsoft VS Code', 'bin'), log);
	const r = runBat(baseEnv({ LOCALAPPDATA: local }));
	const a = expectedArgs(log);
	check('2. PATH に無ければ %LOCALAPPDATA%\\Programs\\Microsoft VS Code を探す', r.code === 0 && a.ok, `exit=${r.code} args=${a.text}`);
}

// 3. 全ユーザー版(Program Files)の標準の場所にある
{
	const log = path.join(work, 'log3.txt');
	const pf = path.join(work, 'pf3');
	makeStub(path.join(pf, 'Microsoft VS Code', 'bin'), log);
	const r = runBat(baseEnv({ ProgramFiles: pf }));
	const a = expectedArgs(log);
	check('3. Program Files の VSCode も探す', r.code === 0 && a.ok, `exit=${r.code} args=${a.text}`);
}

// 4. VSCode がどこにも無い
{
	const r = runBat(baseEnv());
	check('4. VSCode が無ければ日本語で案内して失敗する', r.code === 1 && r.out.includes('Visual Studio Code が見つかりません') && r.out.includes('code.visualstudio.com'), `exit=${r.code}`);
}

// 5. VSIX が無い
{
	const empty = path.join(work, '空の フォルダ');
	fs.mkdirSync(empty);
	fs.copyFileSync(batSrc, path.join(empty, 'install.bat'));
	const r = runBat(baseEnv(), empty);
	check('5. VSIX が無ければ日本語で案内して失敗する', r.code === 1 && r.out.includes('dxlib-devenv-*.vsix が見つかりません'), `exit=${r.code}`);
}

// 6. code が失敗を返したら、失敗の案内を出す
{
	const stub = path.join(work, 'stub-fail');
	fs.mkdirSync(stub, { recursive: true });
	fs.writeFileSync(path.join(stub, 'code.cmd'), '@echo off\r\necho fake failure\r\nexit /b 1\r\n');
	const r = runBat(baseEnv({ PATH: `${stub};${sys32}` }));
	check('6. code が失敗したら「インストールに失敗しました」と案内し、終了コード 1', r.code === 1 && r.out.includes('インストールに失敗しました') && !r.out.includes('インストールが完了しました'), `exit=${r.code}`);
}

// 7. 本物の VSCode(検証用プロファイル)へ実際にインストールする
{
	if (!realVsix) {
		check('7. 本物の VSCode へインストール', false, 'extension/ に .vsix が無い(先に npm run package)');
	} else {
		const extDir = path.join(work, 'real-extensions');
		const userData = path.join(work, 'real-user-data');
		const realCode = path.join(process.env.LOCALAPPDATA, 'Programs', 'Microsoft VS Code', 'bin', 'code.cmd');
		// 配布フォルダに本物の VSIX を置き、検証用の保存先を足す code.cmd(ラッパー)経由で install.bat を動かす
		const real = path.join(work, 'real-dist');
		fs.mkdirSync(real);
		fs.copyFileSync(batSrc, path.join(real, 'install.bat'));
		fs.copyFileSync(path.join(extRoot, realVsix), path.join(real, realVsix));
		const stub = path.join(work, 'stub-real');
		fs.mkdirSync(stub, { recursive: true });
		fs.writeFileSync(path.join(stub, 'code.cmd'), `@echo off\r\ncall "${realCode}" %* --extensions-dir "${extDir}" --user-data-dir "${userData}"\r\nexit /b %ERRORLEVEL%\r\n`);
		const r = runBat({ ...process.env, PATH: `${stub};${process.env.PATH}` }, real);
		const installed = fs.existsSync(extDir) ? fs.readdirSync(extDir).filter((n) => n.startsWith('mahirocreative.dxlib-devenv-')) : [];
		check('7. 本物の VSCode へインストールできる(検証用プロファイル)', r.code === 0 && installed.length === 1 && r.out.includes('インストールが完了しました'), `exit=${r.code} インストール先=${installed.join(',') || 'なし'}`);
	}
}

console.log(`[phase5] 集計: 失敗 ${ng} 件`);
process.exit(ng > 0 ? 1 : 0);
