import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import * as iconv from 'iconv-lite';
import * as vscode from 'vscode';
import { currentFolder, getConfig } from '../env/environment';
import { inspectSdk } from '../env/sdk';
import { run } from '../util/exec';

type ShaderKind = 'vertex' | 'pixel';

const SOURCE_EXTS = new Set(['.hlsl', '.fx']);
const COPY_EXTS = new Set(['.hlsl', '.hlsli', '.fx', '.fxh', '.h']);

/** ファイル名の末尾で種類を決める: *VS.hlsl → 頂点、*PS.hlsl → ピクセル。 */
function classify(file: string): ShaderKind | undefined {
	const stem = path.basename(file, path.extname(file));
	if (/(^|[_\-.])?VS$/i.test(stem) || /_vs$/i.test(stem)) {
		return 'vertex';
	}
	if (/(^|[_\-.])?PS$/i.test(stem) || /_ps$/i.test(stem)) {
		return 'pixel';
	}
	return undefined;
}

function listFiles(dir: string, exts: Set<string>): string[] {
	if (!fs.existsSync(dir)) {
		return [];
	}
	const out: string[] = [];
	for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
		const p = path.join(dir, entry.name);
		if (entry.isDirectory()) {
			if (entry.name !== 'bin') {
				out.push(...listFiles(p, exts));
			}
		} else if (exts.has(path.extname(entry.name).toLowerCase())) {
			out.push(p);
		}
	}
	return out;
}

/** UTF-8 のソースを CP932 に変換する。CP932 に無い文字があれば位置を返す。 */
function toCp932(text: string): { bytes: Buffer; badChar?: { line: number; ch: string } } {
	const clean = text.replace(/^﻿/, '');
	const bytes = iconv.encode(clean, 'shift_jis');
	const roundTrip = iconv.decode(bytes, 'shift_jis');
	if (roundTrip !== clean) {
		const chars = Array.from(clean);
		const back = Array.from(roundTrip);
		let line = 1;
		for (let i = 0; i < chars.length; i++) {
			if (chars[i] === '\n') {
				line++;
			}
			if (chars[i] !== back[i]) {
				return { bytes, badChar: { line, ch: chars[i] } };
			}
		}
	}
	return { bytes };
}

/**
 * プロジェクトのシェーダーを SDK 付属の ShaderCompiler.exe でコンパイルする。
 * ShaderCompiler は CP932 のソースしか読めないので、一時フォルダに変換して渡す。
 */
export async function compileShaders(output: vscode.OutputChannel): Promise<void> {
	const folder = currentFolder();
	if (!folder) {
		void vscode.window.showWarningMessage('プロジェクトのフォルダが開かれていません。');
		return;
	}
	const sdk = inspectSdk(getConfig<string>('sdkPath', '') || undefined);
	if (!sdk?.shaderCompiler) {
		void vscode.window.showErrorMessage('SDK の Tool\\ShaderCompiler\\ShaderCompiler.exe が見つかりません。DxLib パネルで SDK フォルダを確認してください。');
		return;
	}

	const srcDir = path.join(folder.uri.fsPath, getConfig<string>('shader.sourceDir', 'shaders', folder));
	const outDir = path.join(folder.uri.fsPath, getConfig<string>('shader.outputDir', 'shaders/bin', folder));
	const vsTarget = getConfig<string>('shader.vertexTarget', 'vs_4_0', folder);
	const psTarget = getConfig<string>('shader.pixelTarget', 'ps_4_0', folder);

	const sources = listFiles(srcDir, SOURCE_EXTS);
	if (sources.length === 0) {
		void vscode.window.showWarningMessage(`シェーダーが見つかりません: ${srcDir}`);
		return;
	}

	output.clear();
	output.show(true);
	output.appendLine(`[DxLib] シェーダーをコンパイルします (${sources.length} 本) → ${outDir}`);

	// 1. 一時フォルダへ CP932 で書き出す(include も一緒に)
	const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'dxlib-shader-'));
	let hasError = false;
	for (const file of listFiles(srcDir, COPY_EXTS)) {
		const rel = path.relative(srcDir, file);
		const { bytes, badChar } = toCp932(fs.readFileSync(file, 'utf8'));
		if (badChar) {
			output.appendLine(`NG  ${rel}(${badChar.line}): CP932 で表せない文字「${badChar.ch}」があります。コメントから取り除いてください。`);
			hasError = true;
			continue;
		}
		const dst = path.join(tmp, rel);
		fs.mkdirSync(path.dirname(dst), { recursive: true });
		fs.writeFileSync(dst, bytes);
	}

	// 2. 1 本ずつコンパイル
	fs.mkdirSync(outDir, { recursive: true });
	let ok = 0;
	let skipped = 0;
	for (const file of sources) {
		const rel = path.relative(srcDir, file);
		const kind = classify(file);
		if (!kind) {
			output.appendLine(`--  ${rel}: 名前の末尾が VS でも PS でもないので飛ばしました`);
			skipped++;
			continue;
		}
		const target = kind === 'vertex' ? vsTarget : psTarget;
		const outFile = path.join(outDir, path.basename(file, path.extname(file)) + (kind === 'vertex' ? '.vso' : '.pso'));
		const tmpSrc = path.join(tmp, rel);
		if (!fs.existsSync(tmpSrc)) {
			hasError = true;
			continue;
		}
		const r = await run(sdk.shaderCompiler, [`/T${target}`, `/Fo${outFile}`, tmpSrc], path.dirname(tmpSrc));
		const log = iconv.decode(Buffer.from(r.stdout + r.stderr, 'binary'), 'shift_jis').trim();
		if (r.code !== 0 || !fs.existsSync(outFile)) {
			hasError = true;
			output.appendLine(`NG  ${rel} (${target})`);
			if (log) {
				output.appendLine(log.replace(/^/gm, '      '));
			}
		} else {
			ok++;
			output.appendLine(`OK  ${rel} (${target}) → ${path.relative(folder.uri.fsPath, outFile)}`);
		}
	}

	fs.rmSync(tmp, { recursive: true, force: true });
	const summary = `シェーダーのコンパイル: 成功 ${ok} / 失敗 ${sources.length - ok - skipped} / 対象外 ${skipped}`;
	output.appendLine(`[DxLib] ${summary}`);
	if (hasError) {
		void vscode.window.showErrorMessage(summary + '(出力パネルを確認してください)');
	} else {
		void vscode.window.showInformationMessage(summary);
	}
}

/** 新しいシェーダーの雛形(resources/shaders)。DxLib 3.24f の D3D11 で描画まで確認済み(2026-09-23)。 */
export const SHADER_TEMPLATES = [
	{ id: '2d-ps', label: 'ピクセルシェーダー(2D)', description: 'DrawPrimitive2DToShader 用', file: 'PixelShader2D.hlsl', suffix: 'PS' },
	{ id: '3d-ps', label: 'ピクセルシェーダー(3D)', description: 'DrawPolygon3DToShader 用', file: 'PixelShader3D.hlsl', suffix: 'PS' },
	{ id: '3d-vs', label: '頂点シェーダー(3D)', description: 'DrawPolygon3DToShader 用。2D では使われない', file: 'VertexShader.hlsl', suffix: 'VS' },
] as const;

export type ShaderTemplateInfo = (typeof SHADER_TEMPLATES)[number];

export interface NewShaderArgs {
	kindId: string;
	name: string;
}

export interface NewShaderResult {
	ok: boolean;
	error?: string;
	file?: string;
}

/**
 * 新しいシェーダーファイルを雛形から作る(パネル内フォームの送信先。ダイアログは出さない)。
 * 「プロジェクトのフォルダが開いているか」はコマンドの入口(extension.ts)側で確認済みの前提。
 */
export async function createShaderFile(extensionPath: string, args: NewShaderArgs): Promise<NewShaderResult> {
	const folder = currentFolder();
	if (!folder) {
		return { ok: false, error: 'プロジェクトのフォルダが開かれていません。' };
	}
	const kind = SHADER_TEMPLATES.find((t) => t.id === args.kindId);
	if (!kind) {
		return { ok: false, error: '種類を選んでください。' };
	}
	const base = args.name.trim();
	if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(base)) {
		return { ok: false, error: '名前は英数字とアンダースコアだけで、先頭は英字か _ にしてください。' };
	}
	const dir = path.join(folder.uri.fsPath, getConfig<string>('shader.sourceDir', 'shaders', folder));
	fs.mkdirSync(dir, { recursive: true });
	const name = `${base}${kind.suffix}`;
	const file = path.join(dir, `${name}.hlsl`);
	if (fs.existsSync(file)) {
		return { ok: false, error: `同じ名前のシェーダーが既にあります: ${file}` };
	}
	const template = fs.readFileSync(path.join(extensionPath, 'resources', 'shaders', kind.file), 'utf8').replace(/^﻿/, '');
	fs.writeFileSync(file, '﻿' + template.split('__SHADER_NAME__').join(name), 'utf8'); // BOM 付き UTF-8
	return { ok: true, file };
}
