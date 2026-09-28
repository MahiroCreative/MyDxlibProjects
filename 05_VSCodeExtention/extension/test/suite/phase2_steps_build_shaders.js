// 段階 2: ビルドでシェーダーもコンパイルする(DESIGN.md 9.2 章)と、失敗を「OK」と言わない(9 章)。
// 前提: shaders に TestPS.hlsl と TestVS.hlsl がある(phase2.js のシェーダーの項目で作ったもの)。
const fs = require('fs');
const path = require('path');
const vscode = require('vscode');
const { sleep, waitFor } = require('./report');

module.exports = async function (r, { proj, waitTaskEnd }) {
	const sdir = path.join(proj, 'shaders');
	const bin = path.join(sdir, 'bin');
	const exe = path.join(proj, 'build', 'Debug', 'TestGame.exe');
	const incDir = path.join(sdir, 'inc');
	const common = path.join(incDir, 'Common.hlsli');
	const incShader = path.join(sdir, 'IncA_2DPS.hlsl');
	const incUri = vscode.Uri.file(incShader);
	const mtime = (f) => (fs.existsSync(f) ? fs.statSync(f).mtimeMs : 0);
	const build = () => waitTaskEnd(vscode.commands.executeCommand('dxlib.build'), 180000);
	const shaderDiags = () => vscode.languages.getDiagnostics(incUri).filter((d) => d.source === 'ShaderCompiler');
	const out = {
		ps: path.join(bin, 'TestPS.pso'),
		vs: path.join(bin, 'TestVS.vso'),
		inc: path.join(bin, 'IncA_2DPS.pso'),
	};
	const bomUtf8 = (f, text) => fs.writeFileSync(f, '﻿' + text, 'utf8');
	const incSource = (body) =>
		[
			'// #include を使うシェーダー(ビルドでのコンパイルの確認用)',
			'#include "inc/Common.hlsli"',
			'float4 main(float4 pos : SV_POSITION) : SV_TARGET0',
			'{',
			`\treturn ${body};`,
			'}',
			'',
		].join('\r\n');

	try {
		await r.step('ビルド: シェーダーもコンパイルされる(.pso と .vso ができる)', async () => {
			fs.rmSync(bin, { recursive: true, force: true });
			fs.mkdirSync(incDir, { recursive: true });
			bomUtf8(common, '// 共通の関数\r\nfloat4 Tint(float4 c)\r\n{\r\n\treturn c;\r\n}\r\n');
			bomUtf8(incShader, incSource('Tint(pos)'));
			const res = await build();
			const made = Object.values(out).map((f) => fs.existsSync(f));
			return { ok: res.exitCode === 0 && made.every(Boolean), detail: `exit=${res.exitCode} TestPS=${made[0]} TestVS=${made[1]} IncA_2DPS=${made[2]}` };
		});

		await r.step('ビルド: シェーダーを変えていなければ、コンパイルし直さない', async () => {
			const before = Object.values(out).map(mtime);
			await sleep(1100);
			const res = await build();
			const after = Object.values(out).map(mtime);
			const same = before.every((t, i) => t > 0 && t === after[i]);
			return { ok: res.exitCode === 0 && same, detail: `exit=${res.exitCode} 更新時刻が同じ=${same}` };
		});

		await r.step('ビルド: #include しているファイルを変えると、それを読むシェーダーだけコンパイルし直す', async () => {
			const before = { inc: mtime(out.inc), ps: mtime(out.ps) };
			await sleep(1100);
			bomUtf8(common, '// 共通の関数(変更)\r\nfloat4 Tint(float4 c)\r\n{\r\n\treturn c * 0.5f;\r\n}\r\n');
			const res = await build();
			const incRebuilt = mtime(out.inc) > before.inc;
			const psSame = mtime(out.ps) === before.ps;
			return { ok: res.exitCode === 0 && incRebuilt && psSame, detail: `exit=${res.exitCode} IncA_2DPS をコンパイルし直した=${incRebuilt} TestPS はそのまま=${psSame}` };
		});

		await r.step('ビルド: シェーダーのエラーでビルドが止まり、元の .hlsl の正しい行に赤線が付く(前回の .pso は残す)', async () => {
			const psoBefore = mtime(out.inc);
			const exeBefore = mtime(exe);
			await sleep(1100);
			// 5 行目(0 始まりで 4)の undefinedVar がエラー
			bomUtf8(incShader, incSource('Tint(undefinedVar)'));
			const res = await build();
			const diags = await waitFor(() => (shaderDiags().length > 0 ? shaderDiags() : undefined), 10000);
			const onLine = !!diags && diags.some((d) => d.range.start.line === 4 && /undefinedVar/.test(d.message));
			const psoKept = mtime(out.inc) === psoBefore;
			// C++ のビルドまで進んでいないこと(exe が作り直されていない)
			const exeSame = mtime(exe) === exeBefore;
			const msg = diags ? diags.map((d) => `${d.source}:${d.code}:${d.range.start.line + 1}行:${d.message}`).join(' / ') : 'なし';
			return { ok: res.exitCode !== 0 && onLine && psoKept && exeSame, detail: `exit=${res.exitCode} 赤線=${msg} 前回の .pso はそのまま=${psoKept} exe はそのまま=${exeSame}` };
		});

		// ShaderCompiler はエラーでも終了コード 0 で、前回の出力に触らない。以前はそれで「成功」と表示していた
		await r.step('[すべてコンパイル]: 前回の出力が残っていても、エラーのあるシェーダーを「成功」と言わない', async () => {
			const errors = [];
			const infos = [];
			const oe = vscode.window.showErrorMessage;
			const oi = vscode.window.showInformationMessage;
			vscode.window.showErrorMessage = async (m) => (errors.push(m), undefined);
			vscode.window.showInformationMessage = async (m) => (infos.push(m), undefined);
			try {
				await vscode.commands.executeCommand('dxlib.compileShaders');
			} finally {
				vscode.window.showErrorMessage = oe;
				vscode.window.showInformationMessage = oi;
			}
			const ok = fs.existsSync(out.inc) && errors.some((m) => /失敗 1/.test(m)) && infos.length === 0;
			return { ok, detail: `前回の .pso あり=${fs.existsSync(out.inc)} / エラー: ${errors.join(' / ') || 'なし'} / 通知: ${infos.join(' / ') || 'なし'}` };
		});

		await r.step('ビルド: シェーダーのエラーを直すと、ビルドが通り赤線も消える', async () => {
			bomUtf8(incShader, incSource('Tint(pos)'));
			const res = await build();
			const cleared = await waitFor(() => shaderDiags().length === 0, 10000, 200);
			return { ok: res.exitCode === 0 && !!cleared && fs.existsSync(exe), detail: `exit=${res.exitCode} 赤線 ${shaderDiags().length} 件` };
		});
	} finally {
		fs.rmSync(incShader, { force: true });
		fs.rmSync(incDir, { recursive: true, force: true });
		fs.rmSync(out.inc, { force: true });
	}
};
