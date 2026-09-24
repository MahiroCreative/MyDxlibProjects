// 段階 8: 生徒が「C/C++ Extension Pack」を入れてしまった状態でも動くか(DESIGN.md 8 章)。
// Pack には CMake Tools が入り、CMake Tools も C/C++ 拡張の設定プロバイダーになれる。
// プロジェクトは DxLib 拡張を設定プロバイダーに指名しているので、横取りされずに動くはず。
const fs = require('fs');
const path = require('path');
const vscode = require('vscode');
const { Report, sleep, waitFor, includesShadowOf } = require('./report');

const PROVIDER_ID = 'mahirocreative.dxlib-devenv';

/** タスクを実行して終了コードを待つ(phase2.js と同じ)。 */
function waitTaskEnd(startPromise, timeoutMs) {
	return new Promise((resolve) => {
		const timer = setTimeout(() => {
			sub.dispose();
			resolve({ timeout: true });
		}, timeoutMs);
		const sub = vscode.tasks.onDidEndTaskProcess((e) => {
			if (e.execution.task.definition.type === 'dxlib') {
				clearTimeout(timer);
				sub.dispose();
				resolve({ exitCode: e.exitCode });
			}
		});
		Promise.resolve(startPromise).then((ex) => {
			if (!ex) {
				clearTimeout(timer);
				sub.dispose();
				resolve({ noExecution: true });
			}
		});
	});
}

const samePath = (a, b) => !!a && !!b && path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase();

exports.run = async function () {
	const r = new Report('phase8');
	const pack = vscode.extensions.getExtension('ms-vscode.cpptools-extension-pack');
	const cmake = vscode.extensions.getExtension('ms-vscode.cmake-tools');
	r.check('前提: C/C++ Extension Pack と CMake Tools が入っている', !!pack && !!cmake, `pack=${pack ? pack.packageJSON.version : 'なし'} cmake-tools=${cmake ? cmake.packageJSON.version : 'なし'}`);

	const api = await vscode.extensions.getExtension(PROVIDER_ID).activate();
	const proj = vscode.workspace.workspaceFolders[0].uri.fsPath;

	// runTest.js が tasks.json を以前の形(problemMatcher: ["$msCompile"])に戻してから開いている
	await r.step('以前の版の tasks.json は、開いたときに今の形(problemMatcher: [])に直る', async () => {
		const read = () => JSON.parse(fs.readFileSync(path.join(proj, '.vscode', 'tasks.json'), 'utf8')).tasks.map((t) => t.problemMatcher);
		const migrated = await waitFor(() => read().every((pm) => Array.isArray(pm) && pm.length === 0), 10000);
		return { ok: !!migrated, detail: JSON.stringify(read()) };
	});
	const sdk = vscode.workspace.getConfiguration('dxlib').get('sdkPath');

	await r.step('Debug ビルドが通る', async () => {
		const exe = path.join(proj, 'build', 'Debug', 'TestGame.exe');
		fs.rmSync(exe, { force: true });
		const res = await waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);
		return { ok: res.exitCode === 0 && fs.existsSync(exe), detail: `${JSON.stringify(res)} exe=${fs.existsSync(exe)}` };
	});

	const mainUri = vscode.Uri.file(path.join(proj, 'src', 'main.cpp'));
	const openedAt = Date.now();
	const doc = await vscode.workspace.openTextDocument(mainUri);
	await vscode.window.showTextDocument(doc);

	await r.step('補完の設定は DxLib 拡張が渡している(CMake Tools に横取りされない)', async () => {
		const q = await waitFor(() => {
			const last = api.lastIntelliSenseQuery();
			return last && last.at > openedAt ? last : undefined;
		}, 120000, 1000);
		const provider = vscode.workspace.getConfiguration('C_Cpp', folderUri()).get('default.configurationProvider');
		const hasSdk = !!q && includesShadowOf(q.includePath, sdk);
		return { ok: hasSdk && provider === PROVIDER_ID, detail: `C_Cpp.default.configurationProvider=${provider} / ${q ? 'includePath=' + q.includePath.join(' ; ') : '問い合わせが来なかった'} / 状態=${api.intelliSenseState()}` };
	});

	await r.step('main.cpp に赤波線が出ない', async () => {
		await sleep(15000);
		const diags = vscode.languages.getDiagnostics(mainUri).filter((d) => d.severity === vscode.DiagnosticSeverity.Error);
		return { ok: diags.length === 0, detail: diags.length === 0 ? 'エラー 0' : diags.map((d) => `${d.range.start.line + 1}:${d.message}`).join(' / ').slice(0, 300) };
	});

	await r.step('DxLib 関数のホバーに作者コメントが出る', async () => {
		const pos = doc.positionAt(doc.getText().indexOf('DrawString') + 2);
		const text = await waitFor(async () => {
			const hs = await vscode.commands.executeCommand('vscode.executeHoverProvider', mainUri, pos);
			const t = (hs || []).flatMap((h) => h.contents).map((c) => (typeof c === 'string' ? c : c.value)).join('\n');
			return t.includes('**DxLib**') ? t : undefined;
		}, 20000);
		return { ok: !!text, detail: text ? text.split('\n')[0].slice(0, 120) : 'DxLib のホバーが無い' };
	});

	await r.step('デバッグ実行でゲームが起動する', async () => {
		const log = path.join(proj, 'Log.txt');
		for (let i = 0; i < 10; i++) {
			try {
				fs.rmSync(log, { force: true });
				break;
			} catch {
				await sleep(1000);
			}
		}
		let session;
		const sub = vscode.debug.onDidStartDebugSession((s) => (session = s));
		await vscode.commands.executeCommand('dxlib.debug');
		const s = await waitFor(() => session, 180000);
		const logged = await waitFor(() => {
			try {
				return fs.existsSync(log) && fs.readFileSync(log).length > 200;
			} catch {
				return false;
			}
		}, 60000);
		await sleep(2000);
		if (s) {
			await vscode.debug.stopDebugging(s);
		}
		await sleep(3000);
		sub.dispose();
		return { ok: !!s && !!logged, detail: `session=${s ? s.type : 'なし'} Log.txt=${!!logged}` };
	});

	r.finish();
};

function folderUri() {
	return vscode.workspace.workspaceFolders[0].uri;
}
