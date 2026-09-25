// 段階 2: 段階 1 で作ったプロジェクトを開いた状態で、ビルド・ホバー・IntelliSense・シェーダー・デバッグ実行を確かめる。
const fs = require('fs');
const path = require('path');
const vscode = require('vscode');
const { Report, sleep, waitFor, createProjectCapturingOpen } = require('./report');

/** タスクを実行して終了コードを待つ。 */
function waitTaskEnd(startPromise, timeoutMs) {
	return new Promise((resolve) => {
		let execution;
		const timer = setTimeout(() => {
			sub.dispose();
			resolve({ timeout: true });
		}, timeoutMs);
		const sub = vscode.tasks.onDidEndTaskProcess((e) => {
			if (!execution || e.execution === execution || e.execution.task.definition.type === 'dxlib') {
				clearTimeout(timer);
				sub.dispose();
				resolve({ exitCode: e.exitCode });
			}
		});
		Promise.resolve(startPromise).then((ex) => {
			execution = ex;
			if (!ex) {
				clearTimeout(timer);
				sub.dispose();
				resolve({ noExecution: true });
			}
		});
	});
}

function hoverText(hovers) {
	return (hovers || [])
		.flatMap((h) => h.contents)
		.map((c) => (typeof c === 'string' ? c : c.value))
		.join('\n');
}

exports.run = async function () {
	const r = new Report('phase2');
	const ext = vscode.extensions.getExtension('mahirocreative.dxlib-devenv');
	const api = await ext.activate();
	const folder = vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0];
	r.check('プロジェクトのフォルダが開かれている', !!folder, folder && folder.uri.fsPath);

	await r.step('DxLib プロジェクトとして認識される(パネル・ステータスバーの表示条件)', async () => {
		const env = await api.collectEnvironment();
		return { ok: env.isDxLibProject === true, detail: `isDxLibProject=${env.isDxLibProject}` };
	});
	if (!folder) {
		return r.finish();
	}
	const proj = folder.uri.fsPath;
	const cpptools = vscode.extensions.getExtension('ms-vscode.cpptools');
	r.check('C/C++ 拡張がある', !!cpptools, cpptools && cpptools.packageJSON.version);

	// --- タスク -------------------------------------------------------------
	await r.step('dxlib タスクが一覧に出る', async () => {
		const tasks = await vscode.tasks.fetchTasks({ type: 'dxlib' });
		return { ok: tasks.length >= 2, detail: tasks.map((t) => `${t.name}(${t.definition.config}/${t.source})`).join(', ') };
	});

	await r.step('Debug ビルド(パネルの「ビルド」ボタンと同じ経路)', async () => {
		const res = await waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);
		const exe = path.join(proj, 'build', 'Debug', 'TestGame.exe');
		return { ok: res.exitCode === 0 && fs.existsSync(exe), detail: `${JSON.stringify(res)} exe=${fs.existsSync(exe)}` };
	});

	// MSBuild は変更したファイルだけをコンパイルし直す(DESIGN.md 6 章)。
	// 1 回目のビルドのログには main.cpp のコンパイルが出て、変更なしの 2 回目には出ないことを見る
	const buildLog = (config) => {
		const dir = path.join(process.env.DXLIB_USER_DATA_DIR, 'User', 'globalStorage', 'mahirocreative.dxlib-devenv', 'build');
		const f = fs.existsSync(dir) ? fs.readdirSync(dir).find((n) => n.startsWith('TestGame_') && n.endsWith(`_${config}.log`)) : undefined;
		return f ? fs.readFileSync(path.join(dir, f), 'utf8') : '';
	};
	await r.step('MSBuild: 1 回目は main.cpp をコンパイルし、変更なしで 2 回目にビルドするとコンパイルし直さない', async () => {
		const first = buildLog('debug');
		const res = await waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);
		const second = buildLog('debug');
		const compiledFirst = /^\s*main\.cpp\s*$/m.test(first);
		const compiledSecond = /^\s*main\.cpp\s*$/m.test(second);
		const ok = res.exitCode === 0 && compiledFirst && !compiledSecond && second.includes('TestGame.vcxproj ->');
		return { ok, detail: `1 回目にコンパイル=${compiledFirst} 2 回目にコンパイル=${compiledSecond} 2 回目のログ=${JSON.stringify(second.slice(-200))}` };
	});

	await r.step('Release ビルド', async () => {
		const tasks = await vscode.tasks.fetchTasks({ type: 'dxlib' });
		const t = tasks.find((x) => x.definition.config === 'release');
		if (!t) {
			return { ok: false, detail: 'release タスクが無い' };
		}
		const res = await waitTaskEnd(vscode.tasks.executeTask(t), 180000);
		const exe = path.join(proj, 'build', 'Release', 'TestGame.exe');
		return { ok: res.exitCode === 0 && fs.existsSync(exe), detail: `${JSON.stringify(res)} exe=${fs.existsSync(exe)}` };
	});

	// ビルドエラーの赤線は DxLib 拡張が cl の出力から付ける(source 'cl')。直し始めたら消える(DESIGN.md 6 章)。
	const bad = path.join(proj, 'src', 'Broken.cpp');
	const badUri = vscode.Uri.file(bad);
	const clDiags = () => vscode.languages.getDiagnostics(badUri).filter((x) => x.source === 'cl');
	await r.step('コンパイルエラーが問題パネルに出る(DxLib 拡張が付ける赤線)', async () => {
		fs.writeFileSync(bad, '﻿// わざと壊したファイル\nvoid f() { undefined_symbol(); }\n', 'utf8');
		const res = await waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);
		const diags = await waitFor(() => (clDiags().length > 0 ? clDiags() : undefined), 10000);
		const msg = diags ? diags.map((d) => `${d.source}:${d.code}:${d.range.start.line + 1}行:${d.message}`).join(' / ') : 'なし';
		return { ok: res.exitCode !== 0 && !!diags && diags.some((d) => /識別子/.test(d.message) && d.range.start.line === 1), detail: `exit=${res.exitCode} diag=${msg}` };
	});

	await r.step('エラーを直し始めると、ビルドし直さなくても赤線が消える', async () => {
		const before = clDiags().length;
		const doc = await vscode.workspace.openTextDocument(badUri);
		await vscode.window.showTextDocument(doc);
		const edit = new vscode.WorkspaceEdit();
		const at = doc.getText().indexOf('undefined_symbol();');
		edit.replace(badUri, new vscode.Range(doc.positionAt(at), doc.positionAt(at + 'undefined_symbol();'.length)), '');
		await vscode.workspace.applyEdit(edit);
		const cleared = await waitFor(() => clDiags().length === 0, 5000, 200);
		// 後片づけ: 保存して閉じてから消す(未保存のエディタを残すと後のテストで確認ダイアログが出る)
		await doc.save();
		await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
		fs.rmSync(bad, { force: true });
		return { ok: before > 0 && !!cleared, detail: `直す前 ${before} 件 → 直し始めた後 ${clDiags().length} 件` };
	});

	// --- エディタ: ホバー と IntelliSense ------------------------------------
	const mainUri = vscode.Uri.file(path.join(proj, 'src', 'main.cpp'));
	const doc = await vscode.workspace.openTextDocument(mainUri);
	await vscode.window.showTextDocument(doc);

	await r.step('DxLib 関数のホバーに作者コメントが出る', async () => {
		const offset = doc.getText().indexOf('DrawString');
		const pos = doc.positionAt(offset + 2);
		const text = await waitFor(async () => {
			const t = hoverText(await vscode.commands.executeCommand('vscode.executeHoverProvider', mainUri, pos));
			return t.includes('**DxLib**') ? t : undefined;
		}, 20000);
		return { ok: !!text, detail: text ? text.split('\n').slice(0, 3).join(' ⏎ ').slice(0, 200) : 'DxLib のホバーが無い' };
	});

	// DxLib 3.24f には宣言が 4 つ以上ある関数が無く、「残り n 件」のリンクはどの関数でも出ない(2026-09-24 確認)。
	// ここでは別ファイルでもホバーとリファレンスへのリンクが出ることを確かめる(以前の項目名は中身と合っていなかった)。
	await r.step('別の .cpp でも DxLib のホバーと「リファレンスを開く」が出る', async () => {
		fs.writeFileSync(path.join(proj, 'src', 'Probe.cpp'), '\uFEFF#include "DxLib.h"\nvoid probe() { DrawGraph(0, 0, 0, TRUE); LoadGraph("a.png"); }\n', 'utf8');
		const uri = vscode.Uri.file(path.join(proj, 'src', 'Probe.cpp'));
		const d = await vscode.workspace.openTextDocument(uri);
		const pos = d.positionAt(d.getText().indexOf('LoadGraph') + 2);
		const t = hoverText(await vscode.commands.executeCommand('vscode.executeHoverProvider', uri, pos));
		return { ok: t.includes('**DxLib**') && t.includes('リファレンスを開く'), detail: t.replace(/\n/g, ' ⏎ ').slice(0, 300) };
	});

	await r.step('IntelliSense の設定プロバイダーに問い合わせが来る', async () => {
		const st = await waitFor(() => (api.intelliSenseState() === 'ready' ? 'ready' : undefined), 120000, 1000);
		return { ok: st === 'ready', detail: api.intelliSenseState() };
	});

	await r.step('C/C++ 拡張のホバーの説明が文字化けせず、正しい関数の説明が出る(ヘッダーの写し)', async () => {
		// 写しを使う前は、CP932 を UTF-8 として読んで化け、しかも 1 行上の関数(ジョイパッドの無効ゾーン)の説明が出ていた
		const uri = vscode.Uri.file(path.join(proj, 'src', 'Probe.cpp'));
		const d = await vscode.workspace.openTextDocument(uri);
		const pos = d.positionAt(d.getText().indexOf('LoadGraph') + 2);
		const cpptoolsText = await waitFor(async () => {
			const hs = (await vscode.commands.executeCommand('vscode.executeHoverProvider', uri, pos)) || [];
			const texts = hs.map((h) => hoverText([h])).filter((t) => !t.includes('**DxLib**'));
			return texts.find((t) => t.includes('LoadGraph'));
		}, 30000, 1000);
		const ok = !!cpptoolsText && cpptoolsText.includes('画像ファイルからグラフィックハンドルを作成する') && !cpptoolsText.includes('�');
		return { ok, detail: cpptoolsText ? cpptoolsText.replace(/\n/g, ' ⏎ ').slice(0, 200) : 'C/C++ 拡張のホバーが無い' };
	});

	await r.step('IntelliSense が DxLib.h を解決できる(赤波線が出ない)', async () => {
		await sleep(15000);
		const diags = vscode.languages.getDiagnostics(mainUri).filter((d) => d.severity === vscode.DiagnosticSeverity.Error);
		return { ok: diags.length === 0, detail: diags.length === 0 ? 'エラー 0' : diags.map((d) => `${d.range.start.line + 1}:${d.message}`).join(' / ').slice(0, 300) };
	});
	fs.rmSync(path.join(proj, 'src', 'Probe.cpp'), { force: true });

	// --- シェーダー ---------------------------------------------------------
	await r.step('シェーダーのコンパイル(日本語コメント入り、BOM 付き UTF-8)', async () => {
		const dir = path.join(proj, 'shaders');
		fs.mkdirSync(dir, { recursive: true });
		fs.writeFileSync(
			path.join(dir, 'TestPS.hlsl'),
			'\uFEFF// テスト用ピクセルシェーダー: 表示・能力・ソフト\nTexture2D g_Tex : register(t0);\nSamplerState g_Smp : register(s0);\nstruct PS_INPUT { float4 Diffuse : COLOR0; float2 Uv : TEXCOORD0; float4 Pos : SV_POSITION; };\nfloat4 main(PS_INPUT i) : SV_TARGET { return g_Tex.Sample(g_Smp, i.Uv) * i.Diffuse; }\n',
			'utf8',
		);
		fs.writeFileSync(
			path.join(dir, 'TestVS.hlsl'),
			'\uFEFF// テスト用頂点シェーダー\nstruct VS_IN { float3 Pos : POSITION; float4 Col : COLOR0; };\nstruct VS_OUT { float4 Col : COLOR0; float4 Pos : SV_POSITION; };\nVS_OUT main(VS_IN i) { VS_OUT o; o.Pos = float4(i.Pos, 1); o.Col = i.Col; return o; }\n',
			'utf8',
		);
		await vscode.commands.executeCommand('dxlib.compileShaders');
		const pso = path.join(dir, 'bin', 'TestPS.pso');
		const vso = path.join(dir, 'bin', 'TestVS.vso');
		return { ok: fs.existsSync(pso) && fs.existsSync(vso), detail: `pso=${fs.existsSync(pso)} vso=${fs.existsSync(vso)}` };
	});

	await r.step('CP932 に無い文字(絵文字)をエラーにする', async () => {
		const bad = path.join(proj, 'shaders', 'EmojiPS.hlsl');
		fs.writeFileSync(bad, '\uFEFF// 😀\nfloat4 main() : SV_TARGET { return 1; }\n', 'utf8');
		await vscode.commands.executeCommand('dxlib.compileShaders');
		const made = fs.existsSync(path.join(proj, 'shaders', 'bin', 'EmojiPS.pso'));
		fs.rmSync(bad);
		return { ok: !made, detail: made ? '.pso ができてしまった' : '.pso は作られなかった' };
	});

	// --- 新しいシェーダー(パネル内フォーム相当。webview と同じく dxlib.newShaderFile に引数で渡す) ---
	await r.step('新しいシェーダー: 引数なしで呼ぶとパネルのフォームが開く(例外なし)', async () => {
		await vscode.commands.executeCommand('dxlib.newShaderFile');
		return { ok: true, detail: '例外なし(フォームが開いたことはスクリーンショットで別途確認)' };
	});

	await r.step('新しいシェーダー: 不正な入力はエラーで弾かれ、ファイルができない', async () => {
		let msg;
		const orig = vscode.window.showErrorMessage;
		vscode.window.showErrorMessage = async (m) => {
			msg = m;
			return undefined;
		};
		try {
			await vscode.commands.executeCommand('dxlib.newShaderFile', { kindId: '2d-ps', name: '1あいう' });
		} finally {
			vscode.window.showErrorMessage = orig;
		}
		return { ok: !!msg && msg.includes('英数字とアンダースコア'), detail: msg || 'エラーが出なかった' };
	});

	await r.step('新しいシェーダー: 3 種類の雛形からファイルができ、そのままコンパイルが通る', async () => {
		const kinds = [
			{ kindId: '2d-ps', name: 'NewA', file: 'NewA_2DPS.hlsl', out: 'NewA_2DPS.pso' },
			{ kindId: '3d-ps', name: 'NewB', file: 'NewB_3DPS.hlsl', out: 'NewB_3DPS.pso' },
			{ kindId: '3d-vs', name: 'NewC', file: 'NewC_3DVS.hlsl', out: 'NewC_3DVS.vso' },
		];
		for (const k of kinds) {
			await vscode.commands.executeCommand('dxlib.newShaderFile', { kindId: k.kindId, name: k.name });
		}
		await vscode.commands.executeCommand('workbench.action.closeAllEditors');
		await vscode.commands.executeCommand('dxlib.compileShaders');
		let ok = true;
		const details = [];
		for (const k of kinds) {
			const src = path.join(proj, 'shaders', k.file);
			const exists = fs.existsSync(src);
			const bytes = exists ? fs.readFileSync(src) : Buffer.alloc(0);
			const bom = bytes[0] === 0xef && bytes[1] === 0xbb && bytes[2] === 0xbf;
			const replaced = exists && !bytes.toString('utf8').includes('__SHADER_NAME__');
			const compiled = fs.existsSync(path.join(proj, 'shaders', 'bin', k.out));
			ok = ok && exists && bom && replaced && compiled;
			details.push(`${k.file}: 作成=${exists} BOM=${bom} 置換=${replaced} コンパイル=${compiled}`);
			fs.rmSync(src, { force: true });
			fs.rmSync(path.join(proj, 'shaders', 'bin', k.out), { force: true });
		}
		return { ok, detail: details.join(' / ') };
	});

	await r.step('新しいシェーダー: 同じ名前は拒否される', async () => {
		await vscode.commands.executeCommand('dxlib.newShaderFile', { kindId: '2d-ps', name: 'DupTest' });
		await vscode.commands.executeCommand('workbench.action.closeAllEditors');
		let msg;
		const orig = vscode.window.showErrorMessage;
		vscode.window.showErrorMessage = async (m) => {
			msg = m;
			return undefined;
		};
		try {
			await vscode.commands.executeCommand('dxlib.newShaderFile', { kindId: '2d-ps', name: 'DupTest' });
		} finally {
			vscode.window.showErrorMessage = orig;
		}
		fs.rmSync(path.join(proj, 'shaders', 'DupTest_2DPS.hlsl'), { force: true });
		return { ok: !!msg && msg.includes('既にあります'), detail: msg || 'エラーが出なかった' };
	});

	// ファイル名に 2D/3D が入る(2026-09-25 ユーザー決定)。以前は両方 <名前>PS.hlsl で、2 つ目が同名で拒否されていた。
	await r.step('新しいシェーダー: 同じ名前でも 2D 用と 3D 用のピクセルシェーダーを両方作れる', async () => {
		let msg;
		const orig = vscode.window.showErrorMessage;
		vscode.window.showErrorMessage = async (m) => {
			msg = m;
			return undefined;
		};
		try {
			await vscode.commands.executeCommand('dxlib.newShaderFile', { kindId: '2d-ps', name: 'Both' });
			await vscode.commands.executeCommand('dxlib.newShaderFile', { kindId: '3d-ps', name: 'Both' });
		} finally {
			vscode.window.showErrorMessage = orig;
		}
		await vscode.commands.executeCommand('workbench.action.closeAllEditors');
		const files = ['Both_2DPS.hlsl', 'Both_3DPS.hlsl'].map((f) => path.join(proj, 'shaders', f));
		const made = files.map((f) => fs.existsSync(f));
		files.forEach((f) => fs.rmSync(f, { force: true }));
		return { ok: made.every(Boolean) && !msg, detail: `Both_2DPS=${made[0]} Both_3DPS=${made[1]} エラー=${msg || 'なし'}` };
	});

	// --- エクスプローラーの右クリック(DESIGN.md 3.1 章)。VSCode がメニューから渡すのと同じ引数で呼ぶ ---
	// 前の項目で shaders に TestPS.hlsl と TestVS.hlsl がある
	{
		const sdir = path.join(proj, 'shaders');
		const bin = path.join(sdir, 'bin');
		const outOf = (name) => path.join(bin, name);
		const clearBin = () => fs.rmSync(bin, { recursive: true, force: true });
		const captureWarn = async (fn) => {
			const warns = [];
			const ow = vscode.window.showWarningMessage;
			vscode.window.showWarningMessage = async (m) => (warns.push(m), undefined);
			try {
				await fn();
			} finally {
				vscode.window.showWarningMessage = ow;
			}
			return warns;
		};

		await r.step('右クリック「このシェーダーをコンパイル」: 選んだ 1 本だけがコンパイルされる', async () => {
			clearBin();
			const uri = vscode.Uri.file(path.join(sdir, 'TestPS.hlsl'));
			await vscode.commands.executeCommand('dxlib.compileShaderFile', uri, [uri]);
			const ps = fs.existsSync(outOf('TestPS.pso'));
			const vs = fs.existsSync(outOf('TestVS.vso'));
			return { ok: ps && !vs, detail: `TestPS.pso=${ps} TestVS.vso=${vs}(できないのが正しい)` };
		});

		await r.step('右クリック「このシェーダーをコンパイル」: 複数選ぶと選んだものが全部コンパイルされる', async () => {
			clearBin();
			const a = vscode.Uri.file(path.join(sdir, 'TestPS.hlsl'));
			const b = vscode.Uri.file(path.join(sdir, 'TestVS.hlsl'));
			await vscode.commands.executeCommand('dxlib.compileShaderFile', a, [a, b]);
			const ps = fs.existsSync(outOf('TestPS.pso'));
			const vs = fs.existsSync(outOf('TestVS.vso'));
			return { ok: ps && vs, detail: `TestPS.pso=${ps} TestVS.vso=${vs}` };
		});

		await r.step('右クリック「このシェーダーをコンパイル」: シェーダーのフォルダの外のファイルは案内だけで何もしない', async () => {
			clearBin();
			const outside = path.join(proj, 'src', 'Outside_2DPS.hlsl');
			fs.copyFileSync(path.join(sdir, 'TestPS.hlsl'), outside);
			const uri = vscode.Uri.file(outside);
			let warns;
			try {
				warns = await captureWarn(() => vscode.commands.executeCommand('dxlib.compileShaderFile', uri, [uri]));
			} finally {
				fs.rmSync(outside, { force: true });
			}
			const made = fs.existsSync(outOf('Outside_2DPS.pso'));
			return { ok: !made && warns.some((m) => m.includes('中のファイルだけコンパイルできます')), detail: `pso=${made} / ${warns.join(' / ')}` };
		});

		await r.step('右クリック「新しいシェーダーを作成」: フォルダから呼ぶとファイルは作らずフォームを開く(エラーなし)', async () => {
			const before = fs.readdirSync(sdir).filter((f) => f.endsWith('.hlsl')).sort().join(',');
			let err;
			const oe = vscode.window.showErrorMessage;
			vscode.window.showErrorMessage = async (m) => ((err = m), undefined);
			try {
				await vscode.commands.executeCommand('dxlib.newShaderFile', vscode.Uri.file(sdir));
			} finally {
				vscode.window.showErrorMessage = oe;
			}
			const after = fs.readdirSync(sdir).filter((f) => f.endsWith('.hlsl')).sort().join(',');
			return { ok: before === after && !err, detail: `ファイルの増減なし=${before === after} エラー=${err || 'なし'}(フォームが開いたことはスクリーンショットで確認)` };
		});
	}

	// --- HLSL の自動整形(clang-format を C/C++ 拡張から借用) ----------------
	await r.step('HLSL ファイルの保存時整形(タブ・Allman ブレース)', async () => {
		const dir = path.join(proj, 'shaders');
		fs.mkdirSync(dir, { recursive: true });
		const file = path.join(dir, 'MessyPS.hlsl');
		fs.writeFileSync(file, '\uFEFFfloat4 main() : SV_TARGET {\nif(true){\nreturn float4(1,1,1,1);\n}\nreturn 0;\n}\n', 'utf8');
		const uri = vscode.Uri.file(file);
		const d = await vscode.workspace.openTextDocument(uri);
		await vscode.window.showTextDocument(d);
		const edits = await waitFor(async () => {
			const e = await vscode.commands.executeCommand('vscode.executeFormatDocumentProvider', uri, { tabSize: 4, insertSpaces: false });
			return e && e.length > 0 ? e : undefined;
		}, 20000, 1000);
		let after = fs.readFileSync(file, 'utf8');
		if (edits) {
			const we = new vscode.WorkspaceEdit();
			we.set(uri, edits);
			await vscode.workspace.applyEdit(we);
			await d.save();
			after = fs.readFileSync(file, 'utf8');
		}
		fs.rmSync(file, { force: true });
		const ok = /\tif \(true\)\r?\n\t\{/.test(after) && after.includes('\t\treturn');
		return { ok, detail: ok ? '整形された' : after.replace(/\n/g, ' ⏎ ').slice(0, 300) };
	});

	// --- 同梱のテンプレートは、保存時の整形で 1 文字も変わらない(DESIGN.md 10 章) ---
	// C/C++ 拡張は、直す所が無いと空の配列ではなく undefined を返し、「応答しない」と区別できない。
	// そこで、必ず直される行(探り)を末尾に足して整形させ、比べるときは探りの行を除く。
	const PROBE = 'int   fmt_probe  ;';
	const withProbe = (text) => text.replace(/\s*$/, '') + '\r\n\r\n' + PROBE + '\r\n';
	const withoutProbe = (text) => text.replace(/\r?\n\r?\n[^\n]*fmt_probe[^\n]*\r?\n?$/, '\r\n');
	// 整形器の結果(編集)を文書に当てた後の文字列を返す。整形器が応答しないときは undefined。
	const formattedText = async (uri) => {
		const d = await vscode.workspace.openTextDocument(uri);
		await vscode.window.showTextDocument(d);
		const edits = await waitFor(async () => (await vscode.commands.executeCommand('vscode.executeFormatDocumentProvider', uri, { tabSize: 4, insertSpaces: false })) || undefined, 20000, 1000);
		if (!edits) {
			return undefined;
		}
		let text = d.getText();
		const sorted = [...edits].sort((a, b) => d.offsetAt(b.range.start) - d.offsetAt(a.range.start));
		for (const e of sorted) {
			text = text.slice(0, d.offsetAt(e.range.start)) + e.newText + text.slice(d.offsetAt(e.range.end));
		}
		return { before: d.getText(), after: text, edits: edits.length };
	};
	// 探りの行が直されたこと(= 整形器がこのファイルで動いた)と、探り以外が変わらないことを確かめる
	const unchangedExceptProbe = async (uri) => {
		const f = await formattedText(uri);
		if (!f) {
			return { ok: false, detail: '整形器が応答しない' };
		}
		if (!/^int fmt_probe;\r?$/m.test(f.after)) {
			return { ok: false, detail: `探りの行が直されていない(整形器が動いていない): ${f.after.slice(-60)}` };
		}
		const before = withoutProbe(f.before);
		const after = withoutProbe(f.after);
		return before === after ? { ok: true, detail: '変化なし' } : { ok: false, detail: firstDiff(before, after) || '末尾が変わった' };
	};
	const firstDiff = (a, b) => {
		const la = a.split(/\r?\n/);
		const lb = b.split(/\r?\n/);
		const i = la.findIndex((l, k) => l !== lb[k]);
		return i < 0 ? '' : `${i + 1} 行目: ${JSON.stringify(la[i])} → ${JSON.stringify(lb[i])}`;
	};
	const closeAndRemove = async (files) => {
		await vscode.commands.executeCommand('workbench.action.closeAllEditors');
		for (const f of files) {
			fs.rmSync(f, { force: true });
		}
	};

	await r.step('整形: C++ の最小テンプレートは、どの長さのプロジェクト名でも整形で変わらない', async () => {
		const tpl = fs.readFileSync(path.join(ext.extensionPath, 'templates', 'minimal', 'src', 'main.cpp'), 'utf8').replace(/^﻿/, '');
		const files = [];
		const details = [];
		let ok = true;
		try {
			// 対照: 崩した C++ は整形で変わる(整形器が本当に動いていることの確認)
			const messy = path.join(proj, 'src', 'FmtMessy.cpp');
			files.push(messy);
			fs.writeFileSync(messy, '﻿int f(){\nreturn 0;}\n', 'utf8');
			const m = await formattedText(vscode.Uri.file(messy));
			const messyOk = !!m && m.after !== m.before;
			ok = ok && messyOk;
			details.push(`対照(崩した C++ は変わる)=${messyOk}`);
			for (const name of ['A', 'MyGame', 'VeryLongProjectName_0123456789']) {
				const file = path.join(proj, 'src', `FmtCheck_${name}.cpp`);
				files.push(file);
				fs.writeFileSync(file, '﻿' + withProbe(tpl.split('__PROJECT_NAME__').join(name)), 'utf8');
				const res = await unchangedExceptProbe(vscode.Uri.file(file));
				ok = ok && res.ok;
				details.push(`${name}: ${res.detail}`);
			}
		} finally {
			await closeAndRemove(files);
		}
		return { ok, detail: details.join(' / ') };
	});

	await r.step('整形: シェーダーの雛形 3 種は整形で変わらない', async () => {
		const kinds = [
			{ kindId: '2d-ps', file: 'FmtA_2DPS.hlsl', name: 'FmtA' },
			{ kindId: '3d-ps', file: 'FmtB_3DPS.hlsl', name: 'FmtB' },
			{ kindId: '3d-vs', file: 'FmtC_3DVS.hlsl', name: 'FmtC' },
		];
		const files = [];
		const details = [];
		let ok = true;
		try {
			// 対照: 崩した HLSL は整形で変わる
			const messy = path.join(proj, 'shaders', 'FmtMessyPS.hlsl');
			files.push(messy);
			fs.writeFileSync(messy, '﻿float4 main() : SV_TARGET {\nreturn 0;}\n', 'utf8');
			const m = await formattedText(vscode.Uri.file(messy));
			const messyOk = !!m && m.after !== m.before;
			ok = ok && messyOk;
			details.push(`対照(崩した HLSL は変わる)=${messyOk}`);
			for (const k of kinds) {
				await vscode.commands.executeCommand('dxlib.newShaderFile', { kindId: k.kindId, name: k.name });
				const file = path.join(proj, 'shaders', k.file);
				files.push(file);
				if (!fs.existsSync(file)) {
					ok = false;
					details.push(`${k.file}: 作成されない`);
					continue;
				}
				// 作成したファイルは VSCode が開いて中身を覚えているので、書き換えずに、探りの行を足した写しを別名で作る
				const probeFile = file.replace(/\.hlsl$/, '_Probe.hlsl');
				files.push(probeFile);
				fs.writeFileSync(probeFile, '﻿' + withProbe(fs.readFileSync(file, 'utf8').replace(/^﻿/, '')), 'utf8');
				const res = await unchangedExceptProbe(vscode.Uri.file(probeFile));
				ok = ok && res.ok;
				details.push(`${k.file}: ${res.detail}`);
			}
		} finally {
			await closeAndRemove(files);
		}
		return { ok, detail: details.join(' / ') };
	});

	// --- ファイルを追加(phase2_steps_new_files.js。DESIGN.md 3.2 章) ---
	await require('./phase2_steps_new_files')(r, { api, proj, withProbe, unchangedExceptProbe, closeAndRemove, waitTaskEnd });

	// --- リファレンス・定義の作成・ワークロード追加(phase2_steps_ref_def_workload.js) ---
	await require('./phase2_steps_ref_def_workload')(r, { api, proj });

	// --- テンプレート(zip ファイル 1 つ = 1 テンプレート。phase2_steps_templates.js。DESIGN.md 8 章) ---
	await require('./phase2_steps_templates')(r, { api });

	// --- 作成後の開き方(2026-09-24 ユーザー決定)。フォルダを開いている窓なら新しい窓で開く ---
	await r.step('作成後の開き方: プロジェクトを開いている窓からなら、新しい窓で開く(今の作業を閉じない)', async () => {
		const loc = path.join(process.env.DXLIB_TEST_WORK, 'open-test');
		fs.rmSync(loc, { recursive: true, force: true });
		fs.mkdirSync(loc, { recursive: true });
		const opened = await createProjectCapturingOpen(vscode, { name: 'OpenTest2', location: loc, templateId: 'builtin:minimal' });
		const made = fs.existsSync(path.join(loc, 'OpenTest2', 'src', 'main.cpp'));
		fs.rmSync(loc, { recursive: true, force: true });
		const o = opened[0];
		const still = vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0].uri.fsPath;
		return {
			ok: made && opened.length === 1 && o.options && o.options.forceNewWindow === true && still && still.toLowerCase() === proj.toLowerCase(),
			detail: `作成=${made} / openFolder ${opened.length} 回 / ${o ? JSON.stringify(o.options) : ''} / この窓のフォルダ=${still}`,
		};
	});

	// --- 作成先の初期値(2026-09-24 ユーザー決定)。前回の作成先が次の初期値になる ---
	// 段階 3(起動し直した後)で、この場所が残っていることを確かめるので、フォルダ自体は消さない。
	await r.step('作成先の初期値: 作成すると、次の初期値が今回の作成先になる', async () => {
		const loc = path.join(process.env.DXLIB_TEST_WORK, 'last-location');
		fs.rmSync(loc, { recursive: true, force: true });
		fs.mkdirSync(loc, { recursive: true });
		const same = (a, b) => !!a && !!b && path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase();
		const before = api.defaultCreateLocation();
		await createProjectCapturingOpen(vscode, { name: 'LastLocTest', location: loc, templateId: 'builtin:minimal' });
		const made = fs.existsSync(path.join(loc, 'LastLocTest', 'src', 'main.cpp'));
		const after = api.defaultCreateLocation();
		fs.rmSync(path.join(loc, 'LastLocTest'), { recursive: true, force: true });
		return { ok: made && !same(before, loc) && same(after, loc), detail: `作成=${made} / 作る前=${before} / 作った後=${after}` };
	});

	// --- 開発の途中で SDK の場所を変える(phase2_steps_sdk_move.js) ----------------
	// 最後に元の SDK へ戻すので、この後のビルドとデバッグ実行は戻したことの確認も兼ねる。
	await require('./phase2_steps_sdk_move')(r, { api, proj, waitTaskEnd });

	// --- デバッグ実行(F5 と同じ経路) ---------------------------------------
	// ここまでの実験(壊れたソース・生成した Foo.h など)の後片づけが漏れていないかを
	// デバッグ実行そのものより先に確かめる。漏れがあると VSCode 側の
	// 「エラーがあります。続行しますか」ダイアログが割り込み、原因が分かりにくくなるため。
	await r.step('デバッグ実行前: クリーンな状態で Debug ビルドが通る', async () => {
		const res = await waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);
		// 削除済みファイルへの古い診断が C/C++ 拡張側に残ることがある(拡張機能の不具合ではない)ので、
		// 今も実在するファイルの診断だけを見る。
		const diags = vscode.languages
			.getDiagnostics()
			.filter(([uri]) => fs.existsSync(uri.fsPath))
			.flatMap(([, ds]) => ds)
			.filter((d) => d.severity === vscode.DiagnosticSeverity.Error);
		return { ok: res.exitCode === 0 && diags.length === 0, detail: `exit=${res.exitCode} diagnostics=${diags.length}` };
	});

	await r.step('デバッグ実行でゲームが起動する', async () => {
		const log = path.join(proj, 'Log.txt');
		// 直前のデバッグ実行のプロセス終了待ちで、Log.txt がまだ使用中のことがあるのでリトライする。
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
		const started = await vscode.commands.executeCommand('dxlib.debug');
		const s = await waitFor(() => session, 180000);
		// 実行中のゲームが Log.txt に書き込んでいる最中は EBUSY で読めないことがある。例外で止めず、読めるまで待つ。
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
		await sleep(3000); // ネイティブプロセスの終了(Log.txt のハンドル解放)を待つ
		sub.dispose();
		return { ok: !!s && !!logged, detail: `session=${s ? s.type + ':' + s.name : 'なし'} startedCmd=${started} Log.txt(プロジェクト直下)=${!!logged}` };
	});

	r.finish();
};
