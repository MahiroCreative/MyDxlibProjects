// 段階 1: フォルダを開かない状態で、環境検出とプロジェクト作成を確かめる。
const fs = require('fs');
const path = require('path');
const vscode = require('vscode');
const { Report, sleep } = require('./report');

exports.run = async function () {
	const r = new Report('phase1');
	const ext = vscode.extensions.getExtension('mahirocreative.dxlib-devenv');
	r.check('拡張機能が読み込まれている', !!ext);
	const api = await ext.activate();
	r.check('activate が検証用の窓口を返す', api && typeof api.createProject === 'function');

	await r.step('環境検出', async () => {
		const env = await api.collectEnvironment();
		const detail = `VS=${env.vs.state} ${env.vs.displayName || ''} / cl=${env.vs.clPath ? 'あり' : 'なし'} / SDK=${env.sdk ? env.sdk.version : 'なし'} ok=${env.sdk && env.sdk.ok} missing=${env.sdk ? env.sdk.missing.join(',') : ''} / shaderCompiler=${env.sdk && env.sdk.shaderCompiler ? 'あり' : 'なし'} / help=${env.sdk && env.sdk.helpDir ? 'あり' : 'なし'} / cpptools=${env.cpptools}`;
		return { ok: env.vs.state === 'ok' && !!env.vs.clPath && env.sdk && env.sdk.ok && !!env.sdk.shaderCompiler && !!env.sdk.helpDir, detail };
	});

	await r.step('不正なプロジェクト名を拒否する', async () => {
		try {
			await api.createProject({ name: '1あいう', location: process.env.DXLIB_TEST_PROJECTS, templateId: 'builtin:minimal' });
			return { ok: false, detail: '例外が出なかった' };
		} catch (e) {
			return { ok: true, detail: e.message };
		}
	});

	let dir;
	await r.step('プロジェクト作成', async () => {
		dir = await api.createProject({ name: 'TestGame', location: process.env.DXLIB_TEST_PROJECTS, templateId: 'builtin:minimal' });
		return { ok: fs.existsSync(dir), detail: dir };
	});

	if (dir) {
		const expect = ['src/main.cpp', '.vscode/tasks.json', '.vscode/launch.json', '.vscode/c_cpp_properties.json', '.vscode/settings.json', '.clang-format', '.gitignore'];
		r.check('生成ファイルがそろっている', expect.every((f) => fs.existsSync(path.join(dir, f))), expect.filter((f) => !fs.existsSync(path.join(dir, f))).join(', ') || '全部あり');
		r.check('template.json はコピーされない', !fs.existsSync(path.join(dir, 'template.json')));
		const main = fs.readFileSync(path.join(dir, 'src', 'main.cpp'));
		r.check('main.cpp は BOM 付き UTF-8', main[0] === 0xef && main[1] === 0xbb && main[2] === 0xbf);
		const text = main.toString('utf8');
		r.check('__PROJECT_NAME__ が置換されている', text.includes('TestGame') && !text.includes('__PROJECT_NAME__'));
		const tasks = JSON.parse(fs.readFileSync(path.join(dir, '.vscode', 'tasks.json'), 'utf8'));
		r.check('tasks.json にパスが書かれていない', !JSON.stringify(tasks).includes(':\\\\'), JSON.stringify(tasks.tasks.map((t) => t.label)));
		await r.step('同名フォルダへの再作成を拒否する', async () => {
			try {
				await api.createProject({ name: 'TestGame', location: process.env.DXLIB_TEST_PROJECTS, templateId: 'builtin:minimal' });
				return { ok: false, detail: '例外が出なかった' };
			} catch (e) {
				return { ok: true, detail: e.message };
			}
		});
	}

	// --- SDK が正しくない間はプロジェクトを作成できない(ユーザー決定 2026-09-23) -------------
	// 「変更」で間違ったフォルダを選ぶと、拒否せず保存してパネルが ✗ になり、作成もできなくなる。
	const validSdk = vscode.workspace.getConfiguration('dxlib').get('sdkPath');
	const wrongSdk = process.env.DXLIB_TEST_WORK; // DxLib.h が無いフォルダ
	const cfgSet = (v) => vscode.workspace.getConfiguration('dxlib').update('sdkPath', v, vscode.ConfigurationTarget.Global);
	const captureMsgs = async (fn) => {
		const got = { error: [], warning: [] };
		const oe = vscode.window.showErrorMessage;
		const ow = vscode.window.showWarningMessage;
		vscode.window.showErrorMessage = async (m) => (got.error.push(m), undefined);
		vscode.window.showWarningMessage = async (m) => (got.warning.push(m), undefined);
		try {
			await fn();
		} finally {
			vscode.window.showErrorMessage = oe;
			vscode.window.showWarningMessage = ow;
		}
		return got;
	};

	await r.step('SDK の選択: DxLib.h が無いフォルダでも保存され、SDK は ✗ になる', async () => {
		const od = vscode.window.showOpenDialog;
		vscode.window.showOpenDialog = async () => [vscode.Uri.file(wrongSdk)];
		let msgs;
		try {
			msgs = await captureMsgs(() => vscode.commands.executeCommand('dxlib.selectSdk'));
		} finally {
			vscode.window.showOpenDialog = od;
		}
		const saved = vscode.workspace.getConfiguration('dxlib').get('sdkPath');
		const env = await api.collectEnvironment();
		// VSCode はドライブ文字を小文字にして返す(C:\ → c:\)ので、大文字小文字を無視して比べる
		const same = path.normalize(String(saved)).toLowerCase() === path.normalize(wrongSdk).toLowerCase();
		const ok = same && env.sdk && env.sdk.ok === false && msgs.error.some((m) => m.includes('DxLib.h が見つかりません') && m.includes('作成できません'));
		return { ok, detail: `保存=${same} sdk.ok=${env.sdk && env.sdk.ok} エラー=${msgs.error[0] || 'なし'}` };
	});

	await r.step('SDK が正しくない間は、コマンドからもプロジェクトを作成できない', async () => {
		const target = path.join(process.env.DXLIB_TEST_PROJECTS, 'ShouldNotExist');
		const msgs = await captureMsgs(() => vscode.commands.executeCommand('dxlib.createProject', { name: 'ShouldNotExist', location: process.env.DXLIB_TEST_PROJECTS, templateId: 'builtin:minimal' }));
		const ok = !fs.existsSync(target) && msgs.warning.some((m) => m.includes('プロジェクトを作成できません'));
		return { ok, detail: `作られていない=${!fs.existsSync(target)} 警告=${msgs.warning[0] || 'なし'}` };
	});

	await r.step('SDK が正しくない間は、API からも作成が拒否される', async () => {
		try {
			await api.createProject({ name: 'ShouldNotExist2', location: process.env.DXLIB_TEST_PROJECTS, templateId: 'builtin:minimal' });
			return { ok: false, detail: '例外が出なかった' };
		} catch (e) {
			return { ok: e.message.includes('DxLib SDK') && !fs.existsSync(path.join(process.env.DXLIB_TEST_PROJECTS, 'ShouldNotExist2')), detail: e.message };
		}
	});

	await cfgSet(validSdk); // 正しい SDK に戻す
	await r.step('正しい SDK に戻すと、作成できる(✓ に戻る)', async () => {
		const env = await api.collectEnvironment();
		const dir2 = await api.createProject({ name: 'BackToOk', location: process.env.DXLIB_TEST_PROJECTS, templateId: 'builtin:minimal' });
		const ok = env.sdk && env.sdk.ok === true && fs.existsSync(dir2);
		fs.rmSync(dir2, { recursive: true, force: true });
		return { ok, detail: `sdk.ok=${env.sdk && env.sdk.ok} 作成=${fs.existsSync(dir2) ? 'あり(削除失敗)' : 'できた'}` };
	});

	// 実際にユーザーが踏んだ不具合の再発防止: フォルダ(= DxLib プロジェクト)を開いていない状態では、
	// C/C++ 拡張は設定プロバイダーに問い合わせてこない。以前はそれでも 120 秒待って「応答しません」と
	// 誤報していた。ここでは実際に 120 秒待ち、誤報にならないことを確かめる(この段階の最後、他の
	// 検証がすべて終わったタイミングで待つので、実行時間への影響は最小限にしている)。
	await r.step('フォルダを開いていない間は C/C++ 拡張の「応答しません」誤報が出ない(120 秒待機)', async () => {
		await sleep(125_000);
		const state = api.intelliSenseState();
		return { ok: state !== 'unresponsive', detail: `intelliSenseState=${state}` };
	});

	r.finish();
};
