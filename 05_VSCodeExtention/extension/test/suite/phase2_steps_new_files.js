// 段階 2 の一部: 「ファイルを追加」(.cpp・.h・クラス・シェーダー。DESIGN.md 3.2 章)。
// 欄・右クリックの「DxLib」メニュー・見出しのボタンは、コマンドを同じ引数(フォルダの Uri / 引数なし)で呼び、欄のフォームが開くことを確かめる。
// フォームの送信は、欄と同じくコマンドに名前と場所を渡して確かめる。
// メニューが出るかどうか(when)は読み取れないので、クリック操作で確かめる(HANDOFF.md 3 章)。
const fs = require('fs');
const path = require('path');
const vscode = require('vscode');

/** 差し替えて自動で答える。必ず元に戻す。 */
async function withStubs(stubs, fn) {
	const saved = stubs.map(([obj, key]) => [obj, key, obj[key]]);
	for (const [obj, key, value] of stubs) {
		obj[key] = value;
	}
	try {
		return await fn();
	} finally {
		for (const [obj, key, value] of saved) {
			obj[key] = value;
		}
	}
}

const hasBom = (file) => {
	const b = fs.readFileSync(file);
	return b[0] === 0xef && b[1] === 0xbb && b[2] === 0xbf;
};

module.exports = async function (r, { api, proj, withProbe, unchangedExceptProbe, closeAndRemove, waitTaskEnd }) {
	const src = path.join(proj, 'src');
	const sub = path.join(src, 'enemy');
	const created = [];
	const track = (...files) => {
		created.push(...files);
		return files;
	};

	try {
		await r.step('ファイルを追加: クラスで .h と .cpp の組ができる(BOM 付き、雛形の中身、.h が開く)', async () => {
			const [h, cpp] = track(path.join(src, 'Player.h'), path.join(src, 'Player.cpp'));
			await vscode.commands.executeCommand('dxlib.newClass', { name: 'Player', folder: src });
			const ht = fs.existsSync(h) ? fs.readFileSync(h, 'utf8') : '';
			const ct = fs.existsSync(cpp) ? fs.readFileSync(cpp, 'utf8') : '';
			const active = vscode.window.activeTextEditor && vscode.window.activeTextEditor.document.uri.fsPath;
			const ok =
				hasBom(h) &&
				hasBom(cpp) &&
				ht.includes('#pragma once') &&
				ht.includes('class Player') &&
				ht.includes('\tPlayer();') &&
				ht.includes('\t~Player();') &&
				ct.includes('#include "DxLib.h"') &&
				ct.includes('#include "Player.h"') &&
				ct.includes('Player::Player()') &&
				ct.includes('Player::~Player()') &&
				!!active &&
				active.toLowerCase() === h.toLowerCase();
			return { ok, detail: `h=${JSON.stringify(ht.slice(0, 80))} cpp=${JSON.stringify(ct.slice(0, 80))} 開いた=${active}` };
		});

		await r.step('ファイルを追加: 同じ名前・不正な名前・src の外は、案内だけでファイルを作らない', async () => {
			const msgs = [];
			const before = fs.readFileSync(path.join(src, 'Player.cpp'), 'utf8');
			const outside = path.join(proj, 'Outside.cpp');
			track(outside, path.join(src, '1Bad.h'));
			await withStubs([[vscode.window, 'showErrorMessage', async (m) => (msgs.push(m), undefined)]], async () => {
				await vscode.commands.executeCommand('dxlib.newCppSource', { name: 'Player', folder: src });
				await vscode.commands.executeCommand('dxlib.newHeader', { name: '1Bad', folder: src });
				await vscode.commands.executeCommand('dxlib.newCppSource', { name: 'Outside', folder: proj });
			});
			const ok =
				msgs.length === 3 &&
				msgs[0].includes('既にあります') &&
				msgs[1].includes('英数字とアンダースコア') &&
				msgs[2].includes('src フォルダの中') &&
				fs.readFileSync(path.join(src, 'Player.cpp'), 'utf8') === before &&
				!fs.existsSync(outside) &&
				!fs.existsSync(path.join(src, '1Bad.h'));
			return { ok, detail: msgs.join(' | ') };
		});

		// 画面上部の入力欄は使わない(生徒が UI と認識しにくい。DESIGN.md 3.2 章)。呼ばれたら記録して NG にする
		const quickInputs = [];
		const noQuickInput = [
			[vscode.window, 'showInputBox', async (o) => (quickInputs.push(`showInputBox ${o && o.title}`), undefined)],
			[vscode.window, 'showQuickPick', async () => (quickInputs.push('showQuickPick'), undefined)],
		];
		const samePath = (a, b) => !!a && !!b && path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase();

		await r.step('ファイルを追加: 右クリック(フォルダの Uri)は、欄のフォームをそのフォルダで開き、ファイルはまだ作らない', async () => {
			fs.mkdirSync(sub, { recursive: true });
			const [h] = track(path.join(sub, 'Enemy.h'));
			await withStubs(noQuickInput, () => vscode.commands.executeCommand('dxlib.newHeader', vscode.Uri.file(sub)));
			const req = api.lastFormRequest();
			const ok = !!req && req.form === 'cpp' && req.kind === 'h' && samePath(req.folder, sub) && req.place === path.join('src', 'enemy') && !fs.existsSync(h) && quickInputs.length === 0;
			return { ok, detail: `フォーム=${JSON.stringify(req)} 作成済み=${fs.existsSync(h)} 上部の入力欄=${quickInputs.join(',') || 'なし'}` };
		});

		await r.step('ファイルを追加: フォームの送信(名前と場所)で、そのフォルダに作る', async () => {
			const h = path.join(sub, 'Enemy.h');
			const req = api.lastFormRequest();
			await vscode.commands.executeCommand('dxlib.newHeader', { name: 'Enemy', folder: req && req.folder });
			return { ok: fs.existsSync(h) && fs.readFileSync(h, 'utf8').includes('#pragma once'), detail: `作成=${fs.existsSync(h)}` };
		});

		await r.step('ファイルを追加: .cpp は、同じフォルダに同じ名前の .h があればその #include も書く', async () => {
			const [cpp] = track(path.join(sub, 'Enemy.cpp'));
			await vscode.commands.executeCommand('dxlib.newCppSource', { name: 'Enemy', folder: sub });
			const t = fs.existsSync(cpp) ? fs.readFileSync(cpp, 'utf8') : '';
			return { ok: t.includes('#include "DxLib.h"') && t.includes('#include "Enemy.h"'), detail: JSON.stringify(t) };
		});

		await r.step('ファイルを追加: 欄・見出しのボタン(引数なし)は、開いているファイルのフォルダ(src の下)でフォームを開く。src の外なら src', async () => {
			// src/enemy のファイルを開いているとき → src/enemy
			await vscode.window.showTextDocument(vscode.Uri.file(path.join(sub, 'Enemy.h')));
			await withStubs(noQuickInput, () => vscode.commands.executeCommand('dxlib.newClass'));
			const a = api.lastFormRequest();
			// src の外(.clang-format)を開いているとき → src
			await vscode.window.showTextDocument(vscode.Uri.file(path.join(proj, '.clang-format')));
			await withStubs(noQuickInput, () => vscode.commands.executeCommand('dxlib.newCppSource'));
			const b = api.lastFormRequest();
			const ok = !!a && a.kind === 'class' && samePath(a.folder, sub) && !!b && b.kind === 'cpp' && samePath(b.folder, src) && b.place === 'src' && quickInputs.length === 0;
			// 後の整形の確認で使う .h を、フォームの送信と同じ経路で作っておく
			track(path.join(sub, 'Item.h'));
			await vscode.commands.executeCommand('dxlib.newHeader', { name: 'Item', folder: sub });
			return { ok, detail: `enemy のとき=${JSON.stringify(a)} / src の外のとき=${JSON.stringify(b)} / 上部の入力欄=${quickInputs.join(',') || 'なし'}` };
		});

		await r.step('ファイルを追加: シェーダーは欄のシェーダーフォームを開き、送信でシェーダーのフォルダに作る', async () => {
			const file = path.join(proj, 'shaders', 'Wave_3DVS.hlsl');
			track(file);
			await withStubs(noQuickInput, () => vscode.commands.executeCommand('dxlib.addShader'));
			const req = api.lastFormRequest();
			const opened = !!req && req.form === 'shader' && !fs.existsSync(file) && quickInputs.length === 0;
			await vscode.commands.executeCommand('dxlib.newShaderFile', { kindId: '3d-vs', name: 'Wave' });
			const ok = opened && fs.existsSync(file) && hasBom(file);
			return { ok, detail: `フォーム=${JSON.stringify(req)} 作成=${fs.existsSync(file)} 上部の入力欄=${quickInputs.join(',') || 'なし'}` };
		});

		await r.step('ファイルを追加: 雛形(.cpp・.h・クラス)は保存時の整形で変わらない', async () => {
			const details = [];
			let ok = true;
			const probes = [];
			try {
				for (const f of [path.join(src, 'Player.h'), path.join(src, 'Player.cpp'), path.join(sub, 'Enemy.cpp'), path.join(sub, 'Item.h')]) {
					// 作ったファイルは開いて中身を覚えられているので、探りの行を足した写しを別名で作る
					const probe = f.replace(/(\.\w+)$/, '_Probe$1');
					probes.push(probe);
					fs.writeFileSync(probe, '﻿' + withProbe(fs.readFileSync(f, 'utf8').replace(/^﻿/, '')), 'utf8');
					const res = await unchangedExceptProbe(vscode.Uri.file(probe));
					ok = ok && res.ok;
					details.push(`${path.basename(f)}: ${res.detail}`);
				}
			} finally {
				await closeAndRemove(probes);
			}
			return { ok, detail: details.join(' / ') };
		});

		await r.step('ファイルを追加: 足したクラスと src の下のフォルダの .cpp も一緒にビルドが通る', async () => {
			await vscode.commands.executeCommand('workbench.action.closeAllEditors');
			for (const n of ['Player.obj', 'Enemy.obj']) {
				fs.rmSync(path.join(proj, 'build', 'Debug', 'obj', n), { force: true });
			}
			const res = await waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);
			const obj = (n) => fs.existsSync(path.join(proj, 'build', 'Debug', 'obj', n));
			return { ok: res.exitCode === 0 && obj('Player.obj') && obj('Enemy.obj'), detail: `${JSON.stringify(res)} Player.obj=${obj('Player.obj')} Enemy.obj=${obj('Enemy.obj')}` };
		});
	} finally {
		await closeAndRemove(created);
		fs.rmSync(sub, { recursive: true, force: true });
	}
};
