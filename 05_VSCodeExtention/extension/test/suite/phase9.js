// 段階 9: Visual Studio で作ったプロジェクトを開く(DESIGN.md 6.1 章)。
// 試験用プロジェクト(test/fixtures/VsGame)は VS 2026 の空のプロジェクトと同じ形:
// .slnx、文字セット Unicode、ファイルを 1 つずつ書く一覧、BOM なし UTF-8 の日本語、ソースはフォルダの直下。
const crypto = require('crypto');
const fs = require('fs');
const path = require('path');
const vscode = require('vscode');
const { Report, waitFor, includesShadowOf } = require('./report');

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

const md5 = (f) => crypto.createHash('md5').update(fs.readFileSync(f)).digest('hex');
const hasBom = (f) => {
	const b = fs.readFileSync(f);
	return b[0] === 0xef && b[1] === 0xbb && b[2] === 0xbf;
};
const samePath = (a, b) => !!a && !!b && path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase();

exports.run = async function () {
	const r = new Report('phase9');
	const api = await vscode.extensions.getExtension('mahirocreative.dxlib-devenv').activate();
	const proj = vscode.workspace.workspaceFolders[0].uri.fsPath;
	const vcx = path.join(proj, 'VsGame.vcxproj');
	const sdk = vscode.workspace.getConfiguration('dxlib').get('sdkPath');
	const vcxBefore = md5(vcx);
	const filtersBefore = fs.readFileSync(`${vcx}.filters`, 'utf8');
	const mainBefore = fs.readFileSync(path.join(proj, 'main.cpp'), 'utf8');

	await r.step('前提: 試験用プロジェクトは BOM なし UTF-8 の日本語を含む(VS 2026 の保存の形)', async () => {
		const b = fs.readFileSync(path.join(proj, 'player.h'));
		return { ok: !hasBom(path.join(proj, 'player.h')) && b.some((x) => x >= 0x80), detail: `BOM=${hasBom(path.join(proj, 'player.h'))}` };
	});

	await r.step('Visual Studio のプロジェクトとして見つかる(DxLib プロジェクトではない。パネルに「使えるようにする」が出る条件)', async () => {
		const env = await api.collectEnvironment();
		return { ok: env.vsProject === 'VsGame.vcxproj' && env.isDxLibProject === false, detail: `vsProject=${env.vsProject} isDxLibProject=${env.isDxLibProject}` };
	});

	let result;
	await r.step('使えるようにする: 成功し、.vscode の一式ができる(dxlib.vsProject・x64・保存時の整形なし)', async () => {
		result = await vscode.commands.executeCommand('dxlib.adoptVsProject', { convert: true });
		const settingsFile = path.join(proj, '.vscode', 'settings.json');
		const settings = fs.existsSync(settingsFile) ? JSON.parse(fs.readFileSync(settingsFile, 'utf8')) : {};
		const ok =
			!!result &&
			result.ok &&
			fs.existsSync(path.join(proj, '.vscode', 'tasks.json')) &&
			settings['dxlib.vsProject'] === 'VsGame.vcxproj' &&
			settings['dxlib.vsPlatform'] === 'x64' &&
			settings['editor.formatOnSave'] === false;
		return { ok, detail: `result=${JSON.stringify(result && { ok: result.ok, error: result.error })} vsProject=${settings['dxlib.vsProject']} platform=${settings['dxlib.vsPlatform']} formatOnSave=${settings['editor.formatOnSave']}` };
	});

	await r.step('使えるようにする: .vcxproj・.filters は書き換えず、この拡張の .vcxproj・.sln・dxlib.props・.clang-format も作らない', async () => {
		await new Promise((res) => setTimeout(res, 3000)); // 設定の変更で動く移行が終わるのを待つ
		const extra = ['VsGame.sln', 'dxlib.props', '.clang-format'].filter((f) => fs.existsSync(path.join(proj, f)));
		const ok = md5(vcx) === vcxBefore && fs.readFileSync(`${vcx}.filters`, 'utf8') === filtersBefore && extra.length === 0 && !fs.readFileSync(vcx, 'utf8').includes('DxLibDevEnv');
		return { ok, detail: `vcxproj 同じ=${md5(vcx) === vcxBefore} 作ってはいけないもの=${extra.join(',') || 'なし'}` };
	});

	await r.step('使えるようにする: BOM なし UTF-8 のソースを BOM 付き UTF-8 にそろえ、中身(日本語)はそのまま', async () => {
		const main = path.join(proj, 'main.cpp');
		const after = fs.readFileSync(main, 'utf8').replace(/^﻿/, '');
		const ok = hasBom(main) && hasBom(path.join(proj, 'player.h')) && after === mainBefore && !!result && result.converted && result.converted.length === 2;
		return { ok, detail: `main.cpp BOM=${hasBom(main)} player.h BOM=${hasBom(path.join(proj, 'player.h'))} 中身が同じ=${after === mainBefore} 変換=${result && result.converted && result.converted.length}` };
	});

	await r.step('使えるようにする: launch.json の exe は MSBuild の出力先(x64\\Debug\\VsGame.exe)', async () => {
		const launch = JSON.parse(fs.readFileSync(path.join(proj, '.vscode', 'launch.json'), 'utf8'));
		const programs = launch.configurations.map((c) => c.program);
		const ok = programs[0] === '${workspaceFolder}/x64/Debug/VsGame.exe' && programs[1] === '${workspaceFolder}/x64/Release/VsGame.exe';
		return { ok, detail: programs.join(' / ') };
	});

	await r.step('使えるようにした後は DxLib プロジェクトとして扱われる', async () => {
		const ok = await waitFor(async () => (await api.collectEnvironment()).isDxLibProject === true, 10000);
		return { ok: !!ok, detail: `isDxLibProject=${!!ok}` };
	});

	const buildLog = () => {
		const dir = path.join(process.env.DXLIB_USER_DATA_DIR, 'User', 'globalStorage', 'mahirocreative.dxlib-devenv', 'build');
		const f = fs.existsSync(dir) ? fs.readdirSync(dir).find((n) => n.startsWith('VsGame_') && n.endsWith('_debug.log')) : undefined;
		return f ? fs.readFileSync(path.join(dir, f), 'utf8') : '';
	};

	await r.step('ビルド: その .vcxproj を MSBuild でそのままビルドでき、警告 C4819 が出ない(BOM をそろえたので)', async () => {
		const res = await waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);
		const exe = path.join(proj, 'x64', 'Debug', 'VsGame.exe');
		const log = buildLog();
		const ok = res.exitCode === 0 && fs.existsSync(exe) && !log.includes('C4819') && log.includes('VsGame.vcxproj ->');
		return { ok, detail: `${JSON.stringify(res)} exe=${fs.existsSync(exe)} C4819=${log.includes('C4819')}` };
	});

	await r.step('補完: .vcxproj の定義(UNICODE)とインクルードの場所(SDK は UTF-8 の写し)を C/C++ 拡張に渡す', async () => {
		const openedAt = Date.now();
		await vscode.window.showTextDocument(await vscode.workspace.openTextDocument(vscode.Uri.file(path.join(proj, 'main.cpp'))));
		const q = await waitFor(() => {
			const last = api.lastIntelliSenseQuery();
			return last && last.at > openedAt ? last : undefined;
		}, 120000, 1000);
		const ok = !!q && q.defines.includes('UNICODE') && q.defines.includes('_UNICODE') && includesShadowOf(q.includePath, sdk);
		return { ok, detail: q ? `defines=${q.defines.join(',')} includePath=${q.includePath.join(' ; ')}` : '問い合わせが来なかった' };
	});

	await r.step('ファイルを足す: 右クリック(プロジェクトのフォルダ)で欄のフォームが開き、作る場所はフォルダの直下', async () => {
		await vscode.commands.executeCommand('dxlib.newClass', vscode.Uri.file(proj));
		const req = api.lastFormRequest();
		return { ok: !!req && req.form === 'cpp' && req.kind === 'class' && samePath(req.folder, proj) && req.place === '.', detail: JSON.stringify(req) };
	});

	await r.step('ファイルを足す: クラスを作ると .vcxproj と .filters の一覧にも書き足され、元の項目は残る', async () => {
		await vscode.commands.executeCommand('dxlib.newClass', { name: 'Enemy', folder: proj });
		const v = fs.readFileSync(vcx, 'utf8');
		const f = fs.readFileSync(`${vcx}.filters`, 'utf8');
		const ok =
			fs.existsSync(path.join(proj, 'Enemy.cpp')) &&
			fs.existsSync(path.join(proj, 'Enemy.h')) &&
			v.includes('<ClCompile Include="Enemy.cpp" />') &&
			v.includes('<ClInclude Include="Enemy.h" />') &&
			v.includes('<ClCompile Include="main.cpp" />') &&
			v.includes('<ClInclude Include="player.h" />') &&
			/<ClCompile Include="Enemy\.cpp">\s*<Filter>ソース ファイル<\/Filter>/.test(f) &&
			/<ClInclude Include="Enemy\.h">\s*<Filter>ヘッダー ファイル<\/Filter>/.test(f);
		return { ok, detail: `vcxproj に Enemy.cpp=${v.includes('Enemy.cpp')} filters のフィルター=${/Enemy\.cpp">\s*<Filter>ソース/.test(f)}` };
	});

	await r.step('ファイルを足す: 足したクラスも一緒にビルドされる', async () => {
		await vscode.commands.executeCommand('workbench.action.closeAllEditors');
		const res = await waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);
		const log = buildLog();
		return { ok: res.exitCode === 0 && /^\s*Enemy\.cpp\s*$/m.test(log), detail: `${JSON.stringify(res)} Enemy.cpp をコンパイル=${/^\s*Enemy\.cpp\s*$/m.test(log)}` };
	});

	r.finish();
};
