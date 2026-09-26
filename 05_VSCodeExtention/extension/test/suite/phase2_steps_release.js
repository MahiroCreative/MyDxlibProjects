// 段階 2 の一部: リリース実行と「配布用にまとめる」(DESIGN.md 3.2・6.2 章)。
const fs = require('fs');
const path = require('path');
const { execFileSync, spawn } = require('child_process');
const vscode = require('vscode');
const { sleep, waitFor } = require('./report');

/** 実行中のゲームが Log.txt に書き込んでいる最中は EBUSY で読めないことがある。読めるまで待つ */
function logWritten(file) {
	try {
		return fs.existsSync(file) && fs.readFileSync(file).length > 200;
	} catch {
		return false;
	}
}

async function removeWithRetry(file) {
	for (let i = 0; i < 10; i++) {
		try {
			fs.rmSync(file, { force: true });
			return;
		} catch {
			await sleep(1000);
		}
	}
}

/**
 * 「配布用にまとめる」の確かめ(段階 2 と段階 9 で共通)。
 * @param opts.asset 試しに置く素材(プロジェクトからの相対)。dist にも同じ並びで入ること
 * @param opts.mustNot dist に入ってはいけないもの(dist の <名前> からの相対)
 */
async function checkPackage(r, { proj, name, asset, mustNot, prefix }) {
	const dist = path.join(proj, 'dist', name);
	const zip = path.join(proj, 'dist', `${name}.zip`);
	let result;

	await r.step(`${prefix}配布用にまとめる: Release でビルドし、dist\\${name}\\ に exe と素材(プロジェクトと同じ並び)が入り、ソースなどは入らない`, async () => {
		fs.mkdirSync(path.dirname(path.join(proj, asset)), { recursive: true });
		fs.writeFileSync(path.join(proj, asset), Buffer.from([0x89, 0x50, 0x4e, 0x47, 1, 2, 3]));
		result = await vscode.commands.executeCommand('dxlib.packageRelease', { reveal: false });
		const bad = mustNot.filter((f) => fs.existsSync(path.join(dist, f)));
		const ok = !!result && result.ok && fs.existsSync(path.join(dist, `${name}.exe`)) && fs.existsSync(path.join(dist, asset)) && bad.length === 0 && !fs.existsSync(path.join(dist, 'Log.txt'));
		return { ok, detail: `result=${JSON.stringify(result && { ok: result.ok, error: result.error, n: result.files && result.files.length })} exe=${fs.existsSync(path.join(dist, `${name}.exe`))} 素材=${fs.existsSync(path.join(dist, asset))} 入ってはいけないもの=${bad.join(',') || 'なし'}` };
	});

	await r.step(`${prefix}配布用にまとめる: 同じ中身の zip を Windows(Expand-Archive)で展開できる`, async () => {
		const out = path.join(proj, 'dist', 'expanded');
		fs.rmSync(out, { recursive: true, force: true });
		execFileSync('powershell.exe', ['-NoProfile', '-Command', `Expand-Archive -LiteralPath '${zip}' -DestinationPath '${out}' -Force`]);
		const ok = fs.existsSync(path.join(out, name, `${name}.exe`)) && fs.existsSync(path.join(out, name, asset));
		fs.rmSync(out, { recursive: true, force: true });
		return { ok, detail: `zip=${fs.existsSync(zip)} 展開して exe と素材=${ok}` };
	});

	await r.step(`${prefix}配布用にまとめる: dist の exe を、dist を作業フォルダにして起動すると動く(Log.txt が dist にできる)`, async () => {
		const log = path.join(dist, 'Log.txt');
		await removeWithRetry(log);
		const child = spawn(path.join(dist, `${name}.exe`), [], { cwd: dist, detached: true, stdio: 'ignore' });
		const logged = await waitFor(() => logWritten(log), 60000);
		try {
			execFileSync('taskkill.exe', ['/PID', String(child.pid), '/T', '/F']);
		} catch {
			// もう終わっている
		}
		await sleep(2000);
		return { ok: !!logged, detail: `Log.txt(dist)=${!!logged}` };
	});

	fs.rmSync(path.join(proj, asset), { force: true });
}

module.exports = async function (r, { proj }) {
	await r.step('リリース実行: Release でビルドし、デバッガーなしで起動する(Log.txt ができる)', async () => {
		const exe = path.join(proj, 'build', 'Release', 'TestGame.exe');
		const log = path.join(proj, 'Log.txt');
		fs.rmSync(exe, { force: true });
		await removeWithRetry(log);
		let session;
		const sub = vscode.debug.onDidStartDebugSession((s) => (session = s));
		await vscode.commands.executeCommand('dxlib.run');
		const s = await waitFor(() => session, 180000);
		const logged = await waitFor(() => logWritten(log), 60000);
		await sleep(2000);
		if (s) {
			await vscode.debug.stopDebugging(s);
		}
		// デバッガーなしの起動は、デバッグの停止ではゲームが閉じないので、ゲームを直接閉じる(残ると後の検証でフォルダを消せない)
		try {
			execFileSync('taskkill.exe', ['/IM', 'TestGame.exe', '/T', '/F'], { stdio: 'ignore' });
		} catch {
			// もう終わっている
		}
		await sleep(3000); // ネイティブプロセスの終了(Log.txt のハンドル解放)を待つ
		sub.dispose();
		const ok = !!s && s.name === 'DxLib: 実行 (Release)' && fs.existsSync(exe) && !!logged;
		return { ok, detail: `session=${s ? s.name : 'なし'} Release の exe=${fs.existsSync(exe)} Log.txt=${!!logged}` };
	});

	// 素材は image\test.png。コンパイル済みのシェーダー(shaders\bin)も入り、shaders のソース(.hlsl)は入らない
	await vscode.commands.executeCommand('dxlib.compileShaders');
	const binDir = path.join(proj, 'shaders', 'bin');
	const aPso = fs.existsSync(binDir) ? fs.readdirSync(binDir).find((n) => /\.(pso|vso)$/.test(n)) : undefined;
	await checkPackage(r, {
		proj,
		name: 'TestGame',
		asset: path.join('image', 'test.png'),
		mustNot: ['src', 'main.cpp', 'TestGame.vcxproj', 'TestGame.sln', 'dxlib.props', '.vscode', 'build', '.clang-format', path.join('shaders', 'TestPS.hlsl')],
		prefix: '',
	});
	await r.step('配布用にまとめる: コンパイル済みのシェーダー(shaders\\bin)は入る', async () => {
		const ok = !!aPso && fs.existsSync(path.join(proj, 'dist', 'TestGame', 'shaders', 'bin', aPso));
		return { ok, detail: `shaders\\bin の ${aPso}=${ok}` };
	});
};

module.exports.checkPackage = checkPackage;
