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
			{ kindId: '2d-ps', name: 'NewA', file: 'NewAPS.hlsl', out: 'NewAPS.pso' },
			{ kindId: '3d-ps', name: 'NewB', file: 'NewBPS.hlsl', out: 'NewBPS.pso' },
			{ kindId: '3d-vs', name: 'NewC', file: 'NewCVS.hlsl', out: 'NewCVS.vso' },
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
		fs.rmSync(path.join(proj, 'shaders', 'DupTestPS.hlsl'), { force: true });
		return { ok: !!msg && msg.includes('既にあります'), detail: msg || 'エラーが出なかった' };
	});

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

	// --- リファレンス・定義の作成・ワークロード追加(phase2_steps_ref_def_workload.js) ---
	await require('./phase2_steps_ref_def_workload')(r, { api, proj });

	// --- テンプレートとして保存(パネル内フォーム相当。webview と同じく dxlib.saveAsTemplate に引数で渡す) ---
	const templatesDir = path.join(process.env.DXLIB_TEST_WORK, 'templates');

	await r.step('テンプレートとして保存: テンプレートフォルダ未設定なら案内が出てフォームは開かない', async () => {
		await vscode.workspace.getConfiguration('dxlib').update('templatesPath', '', vscode.ConfigurationTarget.Global);
		let msg;
		const orig = vscode.window.showWarningMessage;
		vscode.window.showWarningMessage = async (m) => {
			msg = m;
			return undefined;
		};
		try {
			await vscode.commands.executeCommand('dxlib.saveAsTemplate');
		} finally {
			vscode.window.showWarningMessage = orig;
		}
		return { ok: !!msg && msg.includes('テンプレートフォルダが設定されていません'), detail: msg || '案内が出なかった' };
	});

	fs.mkdirSync(templatesDir, { recursive: true });
	await vscode.workspace.getConfiguration('dxlib').update('templatesPath', templatesDir, vscode.ConfigurationTarget.Global);

	await r.step('テンプレートとして保存: 名前が空ならエラーで弾かれる', async () => {
		let msg;
		const orig = vscode.window.showErrorMessage;
		vscode.window.showErrorMessage = async (m) => {
			msg = m;
			return undefined;
		};
		try {
			await vscode.commands.executeCommand('dxlib.saveAsTemplate', { name: '', description: '', substitute: true });
		} finally {
			vscode.window.showErrorMessage = orig;
		}
		return { ok: !!msg && msg.includes('名前を入力'), detail: msg || 'エラーが出なかった' };
	});

	await r.step('テンプレートとして保存: 引数なしで呼ぶとパネルのフォームが開く(例外なし)', async () => {
		await vscode.commands.executeCommand('dxlib.saveAsTemplate');
		return { ok: true, detail: '例外なし(フォームが開いたことはスクリーンショットで別途確認)' };
	});

	await r.step('テンプレートとして保存', async () => {
		await vscode.commands.executeCommand('dxlib.saveAsTemplate', {
			name: 'TestSavedTemplate',
			description: 'テストで保存したテンプレート',
			substitute: true,
		});

		const dest = path.join(templatesDir, 'TestSavedTemplate');
		const tj = path.join(dest, 'template.json');
		const mainCopy = path.join(dest, 'src', 'main.cpp');
		if (!fs.existsSync(tj) || !fs.existsSync(mainCopy)) {
			return { ok: false, detail: `template.json=${fs.existsSync(tj)} main.cpp=${fs.existsSync(mainCopy)}` };
		}
		const meta = JSON.parse(fs.readFileSync(tj, 'utf8'));
		const body = fs.readFileSync(mainCopy, 'utf8');
		const hasVscode = fs.existsSync(path.join(dest, '.vscode'));
		const ok = meta.name === 'TestSavedTemplate' && meta.description.includes('テスト') && body.includes('__PROJECT_NAME__') && !body.includes('TestGame') && !hasVscode;
		return { ok, detail: `name=${meta.name} placeholder=${body.includes('__PROJECT_NAME__')} .vscode 同梱=${hasVscode}` };
	});

	await r.step('テンプレートとして保存: 同じ名前は拒否される', async () => {
		let msg;
		const orig = vscode.window.showErrorMessage;
		vscode.window.showErrorMessage = async (m) => {
			msg = m;
			return undefined;
		};
		try {
			await vscode.commands.executeCommand('dxlib.saveAsTemplate', { name: 'TestSavedTemplate', description: '', substitute: true });
		} finally {
			vscode.window.showErrorMessage = orig;
		}
		return { ok: !!msg && msg.includes('既にあります'), detail: msg || 'エラーが出なかった' };
	});

	await r.step('保存したテンプレートが一覧に出る(フォルダ名順)', async () => {
		const list = api.listTemplates();
		const names = list.map((t) => t.name);
		const found = list.find((t) => t.id === 'ext:TestSavedTemplate');
		return { ok: !!found && names.includes('最小'), detail: names.join(', ') };
	});

	await r.step('保存したテンプレートから別名のプロジェクトを作れる(往復)', async () => {
		const loc = path.join(process.env.DXLIB_TEST_WORK, 'roundtrip');
		fs.rmSync(loc, { recursive: true, force: true });
		fs.mkdirSync(loc, { recursive: true });
		const dir = await api.createProject({ name: 'RoundTrip', location: loc, templateId: 'ext:TestSavedTemplate' });
		const main = fs.readFileSync(path.join(dir, 'src', 'main.cpp'));
		const text = main.toString('utf8');
		const bom = main[0] === 0xef && main[1] === 0xbb && main[2] === 0xbf;
		const ok = bom && text.includes('RoundTrip') && !text.includes('__PROJECT_NAME__') && !text.includes('TestGame') && fs.existsSync(path.join(dir, '.vscode', 'tasks.json'));
		return { ok, detail: `BOM=${bom} RoundTrip=${text.includes('RoundTrip')} 残り=${text.includes('__PROJECT_NAME__') || text.includes('TestGame')}` };
	});

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
