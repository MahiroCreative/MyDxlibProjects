// 段階 7: 制限モード(信頼されていないフォルダ)で DxLib プロジェクトを開いたとき(DESIGN.md 17.4)。
// 信頼の確認を有効にした別プロファイルで開くので、フォルダは信頼されていない状態になる。
// 「信頼する」を押したあとの動き(開き直さずに使えるようになる)は、VSCode に信頼を与えるコマンドが無いため
// 自動では確かめられない。手動確認に入れている。
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');
const vscode = require('vscode');
const { Report, sleep, waitFor } = require('./report');

function screenshot(name) {
	const out = path.join(process.env.DXLIB_TEST_WORK, 'screenshots', `${name}.png`);
	const script = path.join(__dirname, '..', 'screenshot.ps1');
	const output = execFileSync(
		'powershell.exe',
		['-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass', '-File', script, '-UserDataDir', process.env.DXLIB_USER_DATA_DIR, '-OutPath', out],
		{ encoding: 'utf8' },
	);
	return { ok: fs.existsSync(out) && fs.statSync(out).size > 1000, detail: output.trim() };
}

exports.run = async function () {
	const r = new Report('phase7');
	r.check('前提: フォルダが開かれている', !!vscode.workspace.workspaceFolders, vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0].uri.fsPath);
	r.check('前提: 制限モード(信頼されていない)', vscode.workspace.isTrusted === false, `isTrusted=${vscode.workspace.isTrusted}`);

	// DxLib 拡張は VSIX からインストールしたもの(開発中の拡張は信頼の制限を受けないため)
	const ext = vscode.extensions.getExtension('mahirocreative.dxlib-devenv');
	let api;
	await r.step('制限モードでも DxLib 拡張が起動する(止められない)', async () => {
		if (!ext) {
			return { ok: false, detail: '拡張が見つからない(制限モードで無効にされた)' };
		}
		api = await ext.activate();
		return { ok: ext.isActive && !!api, detail: `isActive=${ext.isActive} / ${ext.extensionPath}` };
	});
	if (!api) {
		return r.finish();
	}
	r.check('ビルドなどの機能は起動していない', api.trustedFeaturesActive() === false, `trustedFeaturesActive=${api.trustedFeaturesActive()}`);

	await r.step('ビルド・実行・プロジェクト作成などのコマンドは登録されていない', async () => {
		const all = new Set(await vscode.commands.getCommands(true));
		const shouldNot = ['dxlib.build', 'dxlib.run', 'dxlib.debug', 'dxlib.compileShaders', 'dxlib.createProject', 'dxlib.selectSdk'];
		const found = shouldNot.filter((c) => all.has(c));
		return { ok: found.length === 0 && all.has('dxlib.manageTrust'), detail: `登録されていた: ${found.join(', ') || 'なし'} / manageTrust=${all.has('dxlib.manageTrust')}` };
	});

	await r.step('スクリーンショット: 制限モードのパネル(信頼の案内)', async () => {
		await vscode.commands.executeCommand('workbench.view.extension.dxlib');
		await sleep(3000);
		return screenshot('panel-restricted');
	});

	await r.step('「このフォルダを信頼する」で信頼の管理画面が開く', async () => {
		await vscode.commands.executeCommand('dxlib.manageTrust');
		const tab = await waitFor(() => {
			const t = vscode.window.tabGroups.activeTabGroup.activeTab;
			return t && /Trust|信頼/.test(t.label) ? t : undefined;
		}, 10000);
		await sleep(1500);
		const s = screenshot('trust-editor');
		return { ok: !!tab && s.ok, detail: `タブ=${tab ? tab.label : (vscode.window.tabGroups.activeTabGroup.activeTab || {}).label} / ${s.detail}` };
	});

	r.finish();
};
