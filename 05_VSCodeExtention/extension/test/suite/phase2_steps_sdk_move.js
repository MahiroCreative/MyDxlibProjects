// 段階 2 の一部: 開発の途中で SDK の場所を変える(フォルダを移動して「変更」で指し直す)。
// プロジェクトには SDK のパスを書かないので、指し直すだけで既存のプロジェクトがそのまま使えるはず。
//   1. SDK パッケージを場所 A にコピーし、「変更」で A を指定 → ビルドが通る
//   2. A を B へ移動する(名前変更)→ 再チェックで ✗、ビルドとシェーダーのコンパイルが失敗する
//      (ここで失敗することが、A を実際に使っていた証拠になる)
//   3. 「変更」で B を指定 → ✓、ビルド・シェーダー・IntelliSense が B を使って動く
//   4. 元の SDK の場所に戻す
const fs = require('fs');
const path = require('path');
const vscode = require('vscode');
const { sleep, waitFor, includesShadowOf } = require('./report');

const VC_DIR = 'プロジェクトに追加すべきファイル_VC用';

/** 「変更」ボタンと同じ経路で SDK フォルダを選ぶ。ダイアログは自動で答える。 */
async function selectSdk(dir) {
	const msgs = [];
	const od = vscode.window.showOpenDialog;
	const oi = vscode.window.showInformationMessage;
	const oe = vscode.window.showErrorMessage;
	const ow = vscode.window.showWarningMessage;
	vscode.window.showOpenDialog = async () => [vscode.Uri.file(dir)];
	vscode.window.showInformationMessage = async (m) => (msgs.push(m), undefined);
	vscode.window.showErrorMessage = async (m) => (msgs.push(m), undefined);
	vscode.window.showWarningMessage = async (m) => (msgs.push(m), undefined);
	try {
		await vscode.commands.executeCommand('dxlib.selectSdk');
	} finally {
		vscode.window.showOpenDialog = od;
		vscode.window.showInformationMessage = oi;
		vscode.window.showErrorMessage = oe;
		vscode.window.showWarningMessage = ow;
	}
	return msgs;
}

/** シェーダーのコンパイル。エラーの通知を拾う。 */
async function compileShaders() {
	const errors = [];
	const oe = vscode.window.showErrorMessage;
	vscode.window.showErrorMessage = async (m) => (errors.push(m), undefined);
	try {
		await vscode.commands.executeCommand('dxlib.compileShaders');
	} finally {
		vscode.window.showErrorMessage = oe;
	}
	return errors;
}

/** Windows のパスを大文字小文字を無視して比べる(VSCode はドライブ文字を小文字で返すことがある)。 */
const samePath = (a, b) => !!a && !!b && path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase();

module.exports = async function (r, { api, proj, waitTaskEnd }) {
	const original = vscode.workspace.getConfiguration('dxlib').get('sdkPath');
	const pkgRoot = path.dirname(original);
	const moveRoot = path.join(process.env.DXLIB_TEST_WORK, 'sdk-move');
	// 日本語と空白を含む場所にする(生徒がありがちな置き場所)
	const placeA = path.join(moveRoot, 'DxLib 旧の場所');
	const placeB = path.join(moveRoot, 'DxLib 新しい場所');
	const sdkA = path.join(placeA, VC_DIR);
	const sdkB = path.join(placeB, VC_DIR);
	const exe = path.join(proj, 'build', 'Debug', 'TestGame.exe');
	const pso = path.join(proj, 'shaders', 'bin', 'TestPS.pso');
	const buildDebug = () => waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);

	try {
		const prepared = await r.step('SDK の移動: SDK パッケージを別の場所 A にコピーする', async () => {
			fs.rmSync(moveRoot, { recursive: true, force: true });
			// 派生パス(..\Tool\ShaderCompiler、..\help)も使うので、パッケージの形のままコピーする
			for (const sub of [VC_DIR, 'Tool', 'help']) {
				fs.cpSync(path.join(pkgRoot, sub), path.join(placeA, sub), { recursive: true });
			}
			return { ok: fs.existsSync(path.join(sdkA, 'DxLib.h')), detail: sdkA };
		});
		if (!prepared) {
			return;
		}

		// C/C++ 拡張は C++ のファイルが開いているときしか問い合わせてこない(DESIGN.md 17.3)。
		// 段階 2 の前半の後片づけでエディタは閉じられているので、ここで開き直す。
		const mainUri = vscode.Uri.file(path.join(proj, 'src', 'main.cpp'));
		await vscode.window.showTextDocument(await vscode.workspace.openTextDocument(mainUri));
		/**
		 * 指定時刻より後に C/C++ 拡張から問い合わせが来て、sdk を含むインクルードパスを受け取ったか。
		 * 直前に main.cpp を開いた反応の問い合わせ(切り替え前の設定)が先に来ることがあるので、
		 * 「最初の問い合わせ」ではなく「sdk を含む問い合わせ」が来るまで待つ。来なければ NG。
		 */
		const waitQuery = async (since, sdk) => {
			let last;
			const q = await waitFor(() => {
				last = api.lastIntelliSenseQuery();
				return last && last.at > since && includesShadowOf(last.includePath, sdk) ? last : undefined;
			}, 60000, 1000);
			const seen = last ? `最後の問い合わせ: ${new Date(last.at).toISOString()} includePath=${last.includePath.join(' ; ')}` : '問い合わせが来なかった';
			return { ok: !!q, detail: q ? `includePath=${q.includePath.join(' ; ')}` : seen };
		};

		const selectedAAt = Date.now();
		await r.step('SDK の移動: 「変更」で A を指定すると ✓', async () => {
			const msgs = await selectSdk(sdkA);
			const env = await api.collectEnvironment();
			return { ok: env.sdk && env.sdk.ok && samePath(env.sdkPath, sdkA), detail: `ok=${env.sdk && env.sdk.ok} path=${env.sdkPath} / ${msgs.join(' / ')}` };
		});

		await r.step('SDK の移動: C/C++ 拡張が問い合わせ直し、A のインクルードパスを受け取る', () => waitQuery(selectedAAt, sdkA));

		await r.step('SDK の移動: A で Debug ビルドが通る', async () => {
			fs.rmSync(exe, { force: true });
			const res = await buildDebug();
			return { ok: res.exitCode === 0 && fs.existsSync(exe), detail: `${JSON.stringify(res)} exe=${fs.existsSync(exe)}` };
		});

		const moved = await r.step('SDK の移動: A のフォルダを B へ移動する', async () => {
			// C/C++ 拡張がヘッダーを読んでいる最中だと一時的に移動できないことがあるので、少し待って再試行する
			let last;
			for (let i = 0; i < 20; i++) {
				try {
					fs.renameSync(placeA, placeB);
					return { ok: !fs.existsSync(placeA) && fs.existsSync(path.join(sdkB, 'DxLib.h')), detail: `${placeA} → ${placeB}` };
				} catch (e) {
					last = e;
					await sleep(1000);
				}
			}
			return { ok: false, detail: `移動できない: ${last && last.message}` };
		});
		if (!moved) {
			return;
		}

		await r.step('SDK の移動: 指し直す前は、再チェックで SDK が ✗ になる', async () => {
			await vscode.commands.executeCommand('dxlib.refreshEnvironment');
			const env = await api.collectEnvironment();
			return { ok: env.sdk === undefined || !env.sdk.ok, detail: `ok=${env.sdk && env.sdk.ok} path=${env.sdkPath}` };
		});

		await r.step('SDK の移動: 指し直す前は、ビルドが失敗する(A を使っていた証拠)', async () => {
			fs.rmSync(exe, { force: true });
			const res = await buildDebug();
			return { ok: res.exitCode !== 0 && !fs.existsSync(exe), detail: `${JSON.stringify(res)} exe=${fs.existsSync(exe)}` };
		});

		await r.step('SDK の移動: 指し直す前は、シェーダーのコンパイルが失敗する', async () => {
			fs.rmSync(pso, { force: true });
			const errors = await compileShaders();
			return { ok: !fs.existsSync(pso) && errors.length > 0, detail: `pso=${fs.existsSync(pso)} / ${errors.join(' / ').slice(0, 150)}` };
		});

		const switchedAt = Date.now();
		await r.step('SDK の移動: 「変更」で B を指定すると ✓ に戻る', async () => {
			const msgs = await selectSdk(sdkB);
			const env = await api.collectEnvironment();
			return { ok: env.sdk && env.sdk.ok && env.sdk.version === '3.24f' && samePath(env.sdkPath, sdkB), detail: `ok=${env.sdk && env.sdk.ok} ver=${env.sdk && env.sdk.version} / ${msgs.join(' / ')}` };
		});

		await r.step('SDK の移動: B で Debug ビルドが通る(MSBuild が読む dxlib.props も B を指す)', async () => {
			fs.rmSync(exe, { force: true });
			const res = await buildDebug();
			// SDK の場所は dxlib.props に書く(DESIGN.md 6 章)。Visual Studio で開いたときも同じ場所を使う
			const props = path.join(proj, 'dxlib.props');
			const text = fs.existsSync(props) ? fs.readFileSync(props, 'utf8') : '';
			// 「変更」から保存されるパスはドライブ文字が小文字のことがあるので、大文字小文字を無視して比べる
			const m = text.match(/<DxLibDir>(.*)<\/DxLibDir>/);
			const pointsB = !!m && samePath(m[1], sdkB);
			return { ok: res.exitCode === 0 && fs.existsSync(exe) && pointsB, detail: `${JSON.stringify(res)} exe=${fs.existsSync(exe)} dxlib.props が B を指す=${pointsB}` };
		});

		await r.step('SDK の移動: B でシェーダーのコンパイルが通る', async () => {
			fs.rmSync(pso, { force: true });
			const errors = await compileShaders();
			return { ok: fs.existsSync(pso) && errors.length === 0, detail: `pso=${fs.existsSync(pso)} ${errors.join(' / ').slice(0, 150)}` };
		});

		// 設定が変わると C/C++ 拡張へ再問い合わせを促す(configProvider.invalidate)
		await r.step('SDK の移動: C/C++ 拡張が問い合わせ直し、B のインクルードパスを受け取る', () => waitQuery(switchedAt, sdkB));

		await r.step('SDK の移動: B でも IntelliSense の赤波線が出ない', async () => {
			await sleep(10000);
			const diags = vscode.languages.getDiagnostics(mainUri).filter((d) => d.severity === vscode.DiagnosticSeverity.Error);
			return { ok: diags.length === 0, detail: diags.length === 0 ? 'エラー 0' : diags.map((d) => `${d.range.start.line + 1}:${d.message}`).join(' / ').slice(0, 300) };
		});
	} finally {
		// 元の SDK に戻す。以降の検証(デバッグ実行など)は元の SDK で動く。
		await vscode.workspace.getConfiguration('dxlib').update('sdkPath', original, vscode.ConfigurationTarget.Global);
		await r.step('SDK の移動: 後片づけ(元の SDK に戻し、コピーを消す)', async () => {
			const env = await api.collectEnvironment();
			let removed = false;
			for (let i = 0; i < 20 && !removed; i++) {
				try {
					fs.rmSync(moveRoot, { recursive: true, force: true });
					removed = !fs.existsSync(moveRoot);
				} catch {
					await sleep(1000);
				}
			}
			return { ok: env.sdk && env.sdk.ok && samePath(env.sdkPath, original) && removed, detail: `sdk=${env.sdkPath} コピー削除=${removed}` };
		});
	}
};
