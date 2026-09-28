import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { currentFolder, getConfig } from '../env/environment';
import { inspectSdk } from '../env/sdk';
import { compileShaderSet, listShaderSources } from './shaderCore';

/** シェーダーのフォルダ(設定 dxlib.shader.sourceDir。既定 shaders)。 */
export function shaderSourceDir(folder: vscode.WorkspaceFolder): string {
	return path.join(folder.uri.fsPath, getConfig<string>('shader.sourceDir', 'shaders', folder));
}

/** シェーダーの出力先(設定 dxlib.shader.outputDir。既定 shaders/bin)。 */
export function shaderOutputDir(folder: vscode.WorkspaceFolder): string {
	return path.join(folder.uri.fsPath, getConfig<string>('shader.outputDir', 'shaders/bin', folder));
}

/**
 * プロジェクトのシェーダーを SDK 付属の ShaderCompiler.exe でコンパイルする(DxLib 欄の [すべてコンパイル] と右クリック)。
 * 変わっていないものも含めて、常に全部(only のときはそのファイル)をコンパイルする。本体は shaderCore.ts(ビルドと共通)。
 * only を渡すと、そのファイルだけをコンパイルする(エクスプローラーの右クリック。シェーダーのフォルダの中のものに限る)。
 */
export async function compileShaders(output: vscode.OutputChannel, only?: string[]): Promise<void> {
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

	const srcDir = shaderSourceDir(folder);
	if (only) {
		// include はシェーダーのフォルダから探すので、その外のファイルはコンパイルしない
		const inside = (f: string): boolean => {
			const rel = path.relative(srcDir, f);
			return !!rel && !rel.startsWith('..') && !path.isAbsolute(rel);
		};
		const outside = only.filter((f) => !inside(f));
		if (outside.length > 0) {
			void vscode.window.showWarningMessage(`シェーダーのフォルダ(${path.relative(folder.uri.fsPath, srcDir)})の中のファイルだけコンパイルできます: ${outside.map((f) => path.basename(f)).join(', ')}`);
		}
		only = only.filter(inside);
		if (only.length === 0) {
			return;
		}
	} else if (listShaderSources(srcDir).length === 0) {
		void vscode.window.showWarningMessage(`シェーダーが見つかりません: ${srcDir}`);
		return;
	}

	output.clear();
	output.show(true);
	const r = await compileShaderSet(
		{
			srcDir,
			outDir: shaderOutputDir(folder),
			baseDir: folder.uri.fsPath,
			compiler: sdk.shaderCompiler,
			vsTarget: getConfig<string>('shader.vertexTarget', 'vs_4_0', folder),
			psTarget: getConfig<string>('shader.pixelTarget', 'ps_4_0', folder),
			only,
			reportSkipped: true,
		},
		(line) => output.appendLine(line),
	);
	if (r.total + r.skipped === 0) {
		return;
	}
	const summary = `シェーダーのコンパイル: 成功 ${r.compiled} / 失敗 ${r.failed} / 対象外 ${r.skipped}`;
	output.appendLine(`[DxLib] ${summary}`);
	if (r.failed > 0) {
		void vscode.window.showErrorMessage(summary + '(出力パネルを確認してください)');
	} else {
		void vscode.window.showInformationMessage(summary);
	}
}
/** 新しいシェーダーの雛形(resources/shaders)。DxLib 3.24f の D3D11 で描画まで確認済み(2026-09-23)。 */
export const SHADER_TEMPLATES = [
	{ id: '2d-ps', label: 'ピクセルシェーダー(2D)', description: 'DrawPrimitive2DToShader 用', file: 'PixelShader2D.hlsl', suffix: '_2DPS' },
	{ id: '3d-ps', label: 'ピクセルシェーダー(3D)', description: 'DrawPolygon3DToShader 用', file: 'PixelShader3D.hlsl', suffix: '_3DPS' },
	{ id: '3d-vs', label: '頂点シェーダー(3D)', description: 'DrawPolygon3DToShader 用。2D では使われない', file: 'VertexShader.hlsl', suffix: '_3DVS' },
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
