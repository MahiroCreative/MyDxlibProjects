// 段階 2 の一部: テンプレート(テンプレートファイル .dxtemplate 1 つ = 1 テンプレート。中身は zip。DESIGN.md 8 章)。
// 保存: 欄のフォームの送信と同じく dxlib.saveAsTemplate に引数で渡す。保存先は保存ダイアログ(差し替えて答える)。
// 作成: 作成フォームの「テンプレートファイル (.dxtemplate) を選ぶ」と同じ api.pickTemplateZip と、api.createProject。
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');
const vscode = require('vscode');

async function withStub(obj, key, value, fn) {
	const orig = obj[key];
	obj[key] = value;
	try {
		return await fn();
	} finally {
		obj[key] = orig;
	}
}

/** Windows の標準の機能(PowerShell の Expand-Archive / Compress-Archive)。拡張の zip と Windows の zip が互いに読めるかを見る */
function powershell(script) {
	return execFileSync('powershell.exe', ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-Command', script], { encoding: 'utf8' });
}

const same = (a, b) => !!a && !!b && path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase();

module.exports = async function (r, { api }) {
	const work = process.env.DXLIB_TEST_WORK;
	const tdir = path.join(work, 'templates');
	fs.rmSync(tdir, { recursive: true, force: true });
	fs.mkdirSync(tdir, { recursive: true });
	const saved = path.join(tdir, 'TestSavedTemplate.dxtemplate');

	await r.step('テンプレートとして保存: 名前が空ならエラーで弾かれる(保存ダイアログも出ない)', async () => {
		let msg;
		let dialog = false;
		await withStub(vscode.window, 'showSaveDialog', async () => ((dialog = true), undefined), () =>
			withStub(vscode.window, 'showErrorMessage', async (m) => ((msg = m), undefined), () => vscode.commands.executeCommand('dxlib.saveAsTemplate', { name: '', description: '', substitute: true })),
		);
		return { ok: !!msg && msg.includes('名前を入力') && !dialog, detail: `${msg || 'エラーが出なかった'} ダイアログ=${dialog}` };
	});

	await r.step('テンプレートとして保存: 引数なしで呼ぶと欄のフォームが開く', async () => {
		await vscode.commands.executeCommand('dxlib.saveAsTemplate');
		const req = api.lastFormRequest();
		return { ok: !!req && req.form === 'template', detail: JSON.stringify(req) };
	});

	await r.step('テンプレートとして保存: 保存ダイアログでキャンセルしたら何も作らない', async () => {
		let opts;
		await withStub(vscode.window, 'showSaveDialog', async (o) => ((opts = o), undefined), () =>
			vscode.commands.executeCommand('dxlib.saveAsTemplate', { name: 'TestSavedTemplate', description: '', substitute: true }),
		);
		const files = fs.readdirSync(tdir).filter((f) => /\.(dxtemplate|zip)$/.test(f));
		const ok = !!opts && path.basename(opts.defaultUri.fsPath) === 'TestSavedTemplate.dxtemplate' && !!opts.filters && JSON.stringify(opts.filters).includes('dxtemplate') && files.length === 0;
		return { ok, detail: `ダイアログの初期値=${opts && opts.defaultUri.fsPath} できたファイル=${files.join(',') || 'なし'}` };
	});

	await r.step('テンプレートとして保存: 保存ダイアログで決めた場所に .dxtemplate ができる(拡張子なしで答えても .dxtemplate)', async () => {
		await withStub(vscode.window, 'showSaveDialog', async () => vscode.Uri.file(saved.replace(/\.dxtemplate$/, '')), () =>
			vscode.commands.executeCommand('dxlib.saveAsTemplate', { name: 'TestSavedTemplate', description: 'テストで保存したテンプレート', substitute: true }),
		);
		return { ok: fs.existsSync(saved), detail: `${saved}=${fs.existsSync(saved)}` };
	});

	await r.step('保存した .dxtemplate は中身が zip で、名前を .zip にすれば Windows(Expand-Archive)で展開でき、中身が正しい(template.json、名前が __PROJECT_NAME__、.vscode・build なし)', async () => {
		const out = path.join(tdir, 'expanded');
		fs.rmSync(out, { recursive: true, force: true });
		const asZip = path.join(tdir, 'TestSavedTemplate-copy.zip');
		fs.copyFileSync(saved, asZip); // Expand-Archive は拡張子が .zip のものしか受け付けない
		powershell(`Expand-Archive -LiteralPath '${asZip}' -DestinationPath '${out}' -Force`);
		const tj = path.join(out, 'template.json');
		const main = path.join(out, 'src', 'main.cpp');
		if (!fs.existsSync(tj) || !fs.existsSync(main)) {
			return { ok: false, detail: `template.json=${fs.existsSync(tj)} main.cpp=${fs.existsSync(main)}` };
		}
		const meta = JSON.parse(fs.readFileSync(tj, 'utf8'));
		const body = fs.readFileSync(main, 'utf8');
		// MSBuild / Visual Studio 用のファイル(PC ごとの dxlib.props を含む)も入れない。作るときに作り直す(DESIGN.md 6 章)
		const bad = ['.vscode', 'build', 'Log.txt', 'dxlib.props', 'TestGame.vcxproj', 'TestGame.sln'].filter((n) => fs.existsSync(path.join(out, n)));
		const ok = meta.name === 'TestSavedTemplate' && meta.description.includes('テスト') && body.includes('__PROJECT_NAME__') && !body.includes('TestGame') && bad.length === 0;
		return { ok, detail: `name=${meta.name} placeholder=${body.includes('__PROJECT_NAME__')} 入ってはいけないもの=${bad.join(',') || 'なし'}` };
	});

	await r.step('保存したテンプレートは「最近使ったテンプレート」として一覧に出る(同梱の「最小」が先頭)', async () => {
		const list = api.listTemplates();
		const found = list.find((t) => same(t.source, saved));
		return { ok: !!found && found.name === 'TestSavedTemplate' && list[0].builtin && list[0].name === '最小', detail: list.map((t) => `${t.name}(${t.id})`).join(', ') };
	});

	await r.step('保存したテンプレートから、別名のプロジェクトを作れる(往復)', async () => {
		const loc = path.join(work, 'roundtrip');
		fs.rmSync(loc, { recursive: true, force: true });
		fs.mkdirSync(loc, { recursive: true });
		const dir = await api.createProject({ name: 'RoundTrip', location: loc, templateId: `file:${saved}` });
		const main = fs.readFileSync(path.join(dir, 'src', 'main.cpp'));
		const text = main.toString('utf8');
		const bom = main[0] === 0xef && main[1] === 0xbb && main[2] === 0xbf;
		const ok = bom && text.includes('RoundTrip') && !text.includes('__PROJECT_NAME__') && !text.includes('TestGame') && fs.existsSync(path.join(dir, '.vscode', 'tasks.json')) && !fs.existsSync(path.join(dir, 'template.json')) && fs.existsSync(path.join(dir, 'RoundTrip.vcxproj')) && !fs.existsSync(path.join(dir, 'TestGame.vcxproj'));
		return { ok, detail: `BOM=${bom} RoundTrip=${text.includes('RoundTrip')} 残り=${text.includes('__PROJECT_NAME__') || text.includes('TestGame')}` };
	});

	await r.step('Windows で圧縮した手作りの .zip(Compress-Archive。日本語のファイル名あり)も、選んでプロジェクトを作れる', async () => {
		const srcDir = path.join(tdir, 'handmade');
		fs.rmSync(srcDir, { recursive: true, force: true });
		fs.mkdirSync(path.join(srcDir, 'src'), { recursive: true });
		fs.writeFileSync(path.join(srcDir, 'template.json'), JSON.stringify({ name: '手作り', description: 'Windows で圧縮' }));
		fs.writeFileSync(path.join(srcDir, 'src', 'main.cpp'), '﻿#include "DxLib.h"\n// __PROJECT_NAME__\nint WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return 0; }\n', 'utf8');
		fs.writeFileSync(path.join(srcDir, 'src', '説明.txt'), 'メモ', 'utf8');
		const zip = path.join(tdir, 'Handmade.zip');
		fs.rmSync(zip, { force: true });
		powershell(`Compress-Archive -Path '${path.join(srcDir, '*')}' -DestinationPath '${zip}' -Force`);
		const picked = await api.pickTemplateZip(zip);
		const listed = api.listTemplates().some((t) => same(t.source, zip) && t.name === '手作り');
		const loc = path.join(work, 'roundtrip');
		const dir = await api.createProject({ name: 'FromHandmade', location: loc, templateId: `file:${zip}` });
		const main = fs.readFileSync(path.join(dir, 'src', 'main.cpp'), 'utf8');
		const jp = fs.existsSync(path.join(dir, 'src', '説明.txt'));
		const ok = picked && listed && main.includes('// FromHandmade') && jp;
		return { ok, detail: `選べた=${picked} 一覧=${listed} 置換=${main.includes('// FromHandmade')} 日本語の名前=${jp}` };
	});

	await r.step('template.json の無いファイルは選べない(案内だけで一覧に入らない)', async () => {
		const plain = path.join(tdir, 'Plain.zip');
		fs.rmSync(plain, { force: true });
		const d = path.join(tdir, 'plain');
		fs.mkdirSync(d, { recursive: true });
		fs.writeFileSync(path.join(d, 'a.txt'), 'x');
		powershell(`Compress-Archive -Path '${path.join(d, '*')}' -DestinationPath '${plain}' -Force`);
		let msg;
		const picked = await withStub(vscode.window, 'showErrorMessage', async (m) => ((msg = m), undefined), () => api.pickTemplateZip(plain));
		const listed = api.listTemplates().some((t) => same(t.source, plain));
		return { ok: picked === false && !listed && !!msg && msg.includes('テンプレートのファイルではありません'), detail: `選べた=${picked} 一覧=${listed} 案内=${msg}` };
	});

	await r.step('消えたテンプレートファイルは「最近使ったテンプレート」から外れる', async () => {
		const copy = path.join(tdir, 'WillBeDeleted.dxtemplate');
		fs.copyFileSync(saved, copy);
		await api.pickTemplateZip(copy);
		const before = api.listTemplates().some((t) => same(t.source, copy));
		fs.rmSync(copy, { force: true });
		const after = api.listTemplates().some((t) => same(t.source, copy));
		return { ok: before && !after, detail: `消す前=${before} 消した後=${after}` };
	});
};
