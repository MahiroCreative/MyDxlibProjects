// 段階 6: C/C++ 拡張が DxLib 拡張より後から入る順番(DESIGN.md 17.3)。
// C/C++ 拡張の無い拡張フォルダで起動し、動いている間に C/C++ 拡張を入れる。
// install.bat を VSCode を開いたまま実行したときと同じ順番になる。
//   DXLIB_TEST_LATE_MODE=late : 15 秒待って警告が出ることを確かめてから入れ、回復を確かめる
//   DXLIB_TEST_LATE_MODE=soon : すぐ入れて、警告が出ないことを確かめる
const { spawn } = require('child_process');
const vscode = require('vscode');
const { Report, sleep, waitFor } = require('./report');

const CPPTOOLS_ID = 'ms-vscode.cpptools';
const WARN_DELAY_MS = 15_000; // extension.ts の警告の待ち時間と同じ
const WARN_TEXT = 'C/C++ 拡張(ms-vscode.cpptools)が無効か未インストールです';

/** install.bat と同じく、外から code --install-extension を実行する。 */
function installCpptools() {
	const env = { ...process.env };
	// 拡張ホストから引き継ぐと、Code.exe が Node として動いたり、実行中の窓へ回送されたりする
	delete env.ELECTRON_RUN_AS_NODE;
	delete env.VSCODE_IPC_HOOK_CLI;
	const args = ['/c', process.env.DXLIB_CODE_CLI, '--extensions-dir', process.env.DXLIB_LATE_EXT_DIR, '--user-data-dir', process.env.DXLIB_USER_DATA_DIR, '--install-extension', process.env.DXLIB_CPPTOOLS_VSIX];
	return new Promise((resolve) => {
		const p = spawn('cmd.exe', args, { env, windowsHide: true });
		let out = '';
		p.stdout.on('data', (d) => (out += d));
		p.stderr.on('data', (d) => (out += d));
		p.on('close', (code) => resolve({ code, out: out.trim().split(/\r?\n/).slice(-1)[0] || '' }));
	});
}

exports.run = async function () {
	const mode = process.env.DXLIB_TEST_LATE_MODE;
	const r = new Report(`phase6-${mode}`);
	const warnings = [];
	const ow = vscode.window.showWarningMessage;
	vscode.window.showWarningMessage = async (m) => (warnings.push(String(m)), undefined);
	const cppWarnings = () => warnings.filter((m) => m.includes(WARN_TEXT)).length;
	try {
		const api = await vscode.extensions.getExtension('mahirocreative.dxlib-devenv').activate();
		const t0 = Date.now();
		r.check('開始時: C/C++ 拡張が入っていない', !vscode.extensions.getExtension(CPPTOOLS_ID));
		r.check('開始時: 状態は「未導入」', api.intelliSenseState() === 'missing', api.intelliSenseState());

		if (mode === 'late') {
			// 来ないまま待ち時間を過ぎたら、警告は出る(警告そのものが消えていないことの確認)
			await sleep(WARN_DELAY_MS + 3000);
			r.check('15 秒たっても無ければ警告が 1 回出る', cppWarnings() === 1, `警告 ${cppWarnings()} 回`);
			const env = await api.collectEnvironment();
			r.check('その間、環境欄は「未導入」', env.cpptools === 'missing', env.cpptools);
		}

		const tInstall = Date.now();
		const installing = installCpptools();
		const appeared = await waitFor(() => vscode.extensions.getExtension(CPPTOOLS_ID), 120_000);
		const appearedMs = Date.now() - t0;
		const inst = await installing;
		r.check('C/C++ 拡張を後から入れると VSCode が認識する', !!appeared, `起動から ${appearedMs} ms / 入れ始めから ${Date.now() - tInstall} ms / code の終了コード ${inst.code}: ${inst.out}`);

		const st = await waitFor(() => (api.intelliSenseState() !== 'missing' ? api.intelliSenseState() : undefined), 10_000);
		r.check('状態が「準備中」以降に切り替わる(開き直し不要)', st === 'preparing' || st === 'ready', st || api.intelliSenseState());
		const env = await api.collectEnvironment();
		r.check('環境欄も「未導入」でなくなる', env.cpptools !== 'missing', env.cpptools);

		if (mode === 'soon') {
			// 警告のタイマーが切れる時刻まで待ってから数える
			await sleep(Math.max(0, WARN_DELAY_MS + 3000 - (Date.now() - t0)));
			if (appearedMs < WARN_DELAY_MS - 3000) {
				r.check('15 秒以内に入ったので警告は出ない', cppWarnings() === 0, `警告 ${cppWarnings()} 回 / 認識まで ${appearedMs} ms`);
			} else {
				r.check('15 秒以内に入ったので警告は出ない', false, `判定できない: 認識まで ${appearedMs} ms かかり、待ち時間に近すぎる`);
			}
		} else {
			// C/C++ 拡張は C++ のファイルが開かれるまで問い合わせてこない。開かないまま 120 秒の待ち時間を
			// 過ぎても「応答しません」にならないこと(2026-09-24 に見つかった誤報。実測で 120.9 秒で誤報していた)。
			const cppOpen = vscode.workspace.textDocuments.filter((d) => /\.(c|cc|cpp|cxx|h|hpp|hxx|inl)$/i.test(d.uri.fsPath)).map((d) => d.uri.fsPath);
			r.check('前提: C/C++ のファイルは開いていない', cppOpen.length === 0, cppOpen.join(', ') || 'なし');
			await sleep(125_000);
			r.check('ファイルを開かずに 125 秒待っても「応答しません」にならない', api.intelliSenseState() === 'preparing', api.intelliSenseState());
			const main = vscode.Uri.joinPath(vscode.workspace.workspaceFolders[0].uri, 'src', 'main.cpp');
			await vscode.window.showTextDocument(await vscode.workspace.openTextDocument(main));
			const ready = await waitFor(() => api.intelliSenseState() === 'ready', 120_000, 1000);
			r.check('main.cpp を開くと問い合わせが来て ✓ になる', !!ready, api.intelliSenseState());
			r.check('回復後に警告が増えていない', cppWarnings() === 1, `警告 ${cppWarnings()} 回`);
		}
	} finally {
		vscode.window.showWarningMessage = ow;
	}
	r.finish();
};
