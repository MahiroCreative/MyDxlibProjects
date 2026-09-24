// 段階 3: パネルの見た目を確認するためのスクリーンショット撮影。
// 合否は判定せず、撮れたことだけを記録する。実際の見た目は人(Claude)が画像を見て判断する。
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');
const vscode = require('vscode');
const { Report, sleep } = require('./report');

function screenshot(name) {
	const out = path.join(process.env.DXLIB_TEST_WORK, 'screenshots', `${name}.png`);
	const script = path.join(__dirname, '..', 'screenshot.ps1');
	const output = execFileSync(
		'powershell.exe',
		['-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass', '-File', script, '-UserDataDir', process.env.DXLIB_USER_DATA_DIR, '-OutPath', out],
		{ encoding: 'utf8' },
	);
	return { path: out, output: output.trim(), ok: fs.existsSync(out) && fs.statSync(out).size > 1000 };
}

exports.run = async function () {
	const r = new Report('phase3');
	const ext = vscode.extensions.getExtension('mahirocreative.dxlib-devenv');
	const api = await ext.activate();

	await r.step('DxLib パネルを開く', async () => {
		await vscode.commands.executeCommand('workbench.view.extension.dxlib');
		await sleep(2500); // 環境チェックの再描画を待つ
	});

	await r.step('スクリーンショット: 通常時のパネル', () => {
		const s = screenshot('panel-default');
		return { ok: s.ok, detail: s.output };
	});

	await r.step('スクリーンショット: SDK が正しくないとき(✗ と作成ボタン無効)', async () => {
		const cfg = vscode.workspace.getConfiguration('dxlib');
		const valid = cfg.get('sdkPath');
		try {
			await cfg.update('sdkPath', process.env.DXLIB_TEST_WORK, vscode.ConfigurationTarget.Global); // DxLib.h が無いフォルダ
			await vscode.commands.executeCommand('workbench.view.extension.dxlib');
			await sleep(3000);
			const s = screenshot('panel-sdk-invalid');
			return { ok: s.ok, detail: s.output };
		} finally {
			await cfg.update('sdkPath', valid, vscode.ConfigurationTarget.Global);
		}
	});

	const lastLoc = path.join(process.env.DXLIB_TEST_WORK, 'last-location');
	const same = (a, b) => !!a && !!b && path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase();
	await r.step('作成先の初期値: 起動し直しても、前回(段階 2)の作成先が残っている', async () => {
		const d = api.defaultCreateLocation();
		return { ok: same(d, lastLoc), detail: d };
	});

	await r.step('スクリーンショット: 新規プロジェクト作成フォーム(作成先は前回の場所)', async () => {
		await api.showCreateForm();
		await sleep(1500);
		const s = screenshot('panel-create-form');
		return { ok: s.ok, detail: s.output };
	});

	await r.step('作成先の初期値: 前回の場所が消されていたら、開いているプロジェクトの親フォルダに戻る', async () => {
		fs.rmSync(lastLoc, { recursive: true, force: true });
		const d = api.defaultCreateLocation();
		const parent = path.dirname(vscode.workspace.workspaceFolders[0].uri.fsPath);
		return { ok: same(d, parent), detail: `${d}(期待: ${parent})` };
	});

	await r.step('スクリーンショット: 新しいシェーダーフォーム', async () => {
		await api.showNewShaderForm();
		await sleep(1500);
		const s = screenshot('panel-shader-form');
		return { ok: s.ok, detail: s.output };
	});

	await r.step('スクリーンショット: テンプレートとして保存フォーム', async () => {
		await api.showSaveTemplateForm();
		await sleep(1500);
		const s = screenshot('panel-template-form');
		return { ok: s.ok, detail: s.output };
	});

	r.finish();
};
