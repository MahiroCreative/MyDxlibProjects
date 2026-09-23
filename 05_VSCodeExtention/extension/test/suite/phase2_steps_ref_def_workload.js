// 段階 2 のうち、リファレンス・定義の作成・ワークロード追加の検証(2026-09-23 Opus で作り直し)。
// Sonnet 版の弱点(常に合格・URL の形だけ・導入済み分岐だけ)を直したもの。
const fs = require('fs');
const os = require('os');
const path = require('path');
const { execFileSync } = require('child_process');
const vscode = require('vscode');
const { sleep, waitFor } = require('./report');

async function withStub(obj, key, fn, body) {
	const orig = obj[key];
	obj[key] = fn;
	try {
		return await body();
	} finally {
		obj[key] = orig;
	}
}

module.exports = async function (r, ctx) {
	const { api, proj } = ctx;

	// --- リファレンス -------------------------------------------------------
	await r.step('DxLib リファレンスを開く: ファイルとアンカーが実在する', async () => {
		let opened;
		await withStub(vscode.env, 'openExternal', async (uri) => ((opened = uri), true), () => vscode.commands.executeCommand('dxlib.openReference', 'LoadGraph'));
		if (!opened) {
			return { ok: false, detail: '開かれなかった' };
		}
		const file = opened.fsPath;
		const anchor = opened.fragment;
		const exists = fs.existsSync(file);
		const html = exists ? fs.readFileSync(file, 'utf8') : '';
		// アンカーの位置の宣言が本当に LoadGraph か(別の関数のページに飛ばしていないか)
		const at = html.indexOf(`name="${anchor}"`);
		const near = at >= 0 ? html.slice(at, at + 400) : '';
		const ok = exists && at >= 0 && /LoadGraph\s*\(/.test(near);
		return { ok, detail: `${path.basename(file)}#${anchor} exists=${exists} anchor=${at >= 0} 宣言=${/LoadGraph\s*\(/.test(near)}` };
	});

	await r.step('リファレンスに無い名前は案内だけで例外にならない', async () => {
		let msg;
		await withStub(vscode.window, 'showInformationMessage', async (m) => ((msg = m), undefined), () => vscode.commands.executeCommand('dxlib.openReference', 'NoSuchDxLibFunctionXYZ'));
		return { ok: !!msg && msg.includes('リファレンスの目次にありません'), detail: msg };
	});

	// --- 定義を作成 ---------------------------------------------------------
	await r.step('定義を作成: ヘッダーの宣言から .cpp に雛形ができる', async () => {
		const srcDir = path.join(proj, 'src');
		const hFile = path.join(srcDir, 'Foo.h');
		const cppFile = path.join(srcDir, 'Foo.cpp');
		fs.writeFileSync(hFile, '\uFEFF#pragma once\n\nvoid Foo(int value);\n', 'utf8');
		fs.writeFileSync(cppFile, '\uFEFF#include "Foo.h"\n', 'utf8');

		const cppContent = () => {
			// C/C++ 拡張は編集を未保存のまま残すことがあるので、開いているドキュメントを優先して見る
			const doc = vscode.workspace.textDocuments.find((d) => d.uri.fsPath.toLowerCase() === cppFile.toLowerCase());
			return { text: doc ? doc.getText() : fs.readFileSync(cppFile, 'utf8'), dirty: doc ? doc.isDirty : false, open: !!doc };
		};

		const log = [];
		let result;
		try {
			const hDoc = await vscode.workspace.openTextDocument(vscode.Uri.file(hFile));
			const editor = await vscode.window.showTextDocument(hDoc);
			const offset = hDoc.getText().indexOf('Foo(');
			editor.selection = new vscode.Selection(hDoc.positionAt(offset + 1), hDoc.positionAt(offset + 1));
			// ヘッダーが IntelliSense に解析されるまで待つ
			await sleep(8000);
			for (let i = 0; i < 5 && !result; i++) {
				try {
					await vscode.commands.executeCommand('dxlib.createDefinition');
					log.push(`#${i} 実行 OK`);
				} catch (e) {
					log.push(`#${i} 例外: ${e && e.message ? e.message : e}`);
				}
				const found = await waitFor(() => {
					const c = cppContent();
					return /void\s+Foo\s*\(\s*int\s+value\s*\)\s*\{/.test(c.text) ? c : undefined;
				}, 6000);
				if (found) {
					result = found;
				} else {
					const c = cppContent();
					log.push(`  Foo.cpp open=${c.open} dirty=${c.dirty} len=${c.text.length}`);
					await sleep(3000);
				}
			}
		} finally {
			// 未保存の編集を残したまま閉じると確認ダイアログが出るので、保存してから閉じる
			await vscode.commands.executeCommand('workbench.action.files.saveAll');
			await vscode.commands.executeCommand('workbench.action.closeAllEditors');
			await sleep(1500);
			for (const f of fs.readdirSync(srcDir)) {
				if (/^Foo/i.test(f)) {
					fs.rmSync(path.join(srcDir, f), { force: true });
				}
			}
		}
		const detail = result ? `生成された(open=${result.open} dirty=${result.dirty}): ${result.text.replace(/\uFEFF/g, '').replace(/\n/g, ' ⏎ ')}` : `生成されなかった: ${log.join(' / ')}`;
		return { ok: !!result, detail };
	});

	await r.step('後片づけ後に src が元どおり(main.cpp だけ)', async () => {
		const files = fs.readdirSync(path.join(proj, 'src')).sort();
		return { ok: files.length === 1 && files[0] === 'main.cpp', detail: files.join(', ') };
	});

	// --- C++ ワークロード追加 ---------------------------------------------
	await r.step('ワークロード追加: 導入済みなら案内だけで終わる', async () => {
		let msg;
		await withStub(vscode.window, 'showInformationMessage', async (m) => ((msg = m), undefined), () => vscode.commands.executeCommand('dxlib.addCppWorkload'));
		return { ok: !!msg && msg.includes('既にインストール'), detail: msg || '案内が出なかった' };
	});

	await r.step('ワークロード追加: インストーラーへの引数が割れずに届く(本番と同じ組み立て)', async () => {
		const env = await api.collectEnvironment();
		const installPath = env.vs.installationPath;
		if (!installPath) {
			return { ok: false, detail: 'VS のインストール先が取れない' };
		}
		// 本物の setup.exe の代わりに、受け取った引数を書き出すだけのスクリプトを起動する。
		// 本番との違いは -Verb RunAs(UAC)の代わりに -Wait -NoNewWindow を付けることだけ。
		const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'dxlib-args-'));
		const printer = path.join(tmp, 'printargs.js');
		const out = path.join(tmp, 'out.json');
		fs.writeFileSync(printer, "require('fs').writeFileSync(process.argv[2], JSON.stringify(process.argv.slice(3)));\n");
		const expected = api.workloadInstallArgs(installPath);
		const script = api.buildStartProcessScript(process.execPath, [printer, out, ...expected], '-Wait -NoNewWindow');
		execFileSync('powershell.exe', ['-NoProfile', '-NonInteractive', '-Command', script], { env: { ...process.env, ELECTRON_RUN_AS_NODE: '1' } });
		const got = fs.existsSync(out) ? JSON.parse(fs.readFileSync(out, 'utf8')) : null;
		fs.rmSync(tmp, { recursive: true, force: true });
		const ok = JSON.stringify(got) === JSON.stringify(expected);
		return { ok, detail: ok ? `一致: ${JSON.stringify(got)}` : `期待 ${JSON.stringify(expected)} / 実際 ${JSON.stringify(got)}` };
	});
};
