import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import * as iconv from 'iconv-lite';
import { run } from '../util/exec';

/**
 * シェーダーのコンパイル本体(DESIGN.md 9・9.2 章)。vscode に依存しない。
 * DxLib 欄の [すべてコンパイル](compileShaders.ts)と、ビルド用 bat から呼ぶ buildShaders.ts の両方が使う。
 */

type ShaderKind = 'vertex' | 'pixel';

const SOURCE_EXTS = new Set(['.hlsl', '.fx']);
const COPY_EXTS = new Set(['.hlsl', '.hlsli', '.fx', '.fxh', '.h']);
const INCLUDE_LINE = /^[ \t]*#[ \t]*include[ \t]*[<"]([^>"\r\n]+)[>"]/gm;

export interface ShaderJob {
	/** シェーダーのフォルダ(設定 dxlib.shader.sourceDir)。 */
	srcDir: string;
	/** 出力先(設定 dxlib.shader.outputDir)。 */
	outDir: string;
	/** 表示用の起点(プロジェクトのフォルダ)。 */
	baseDir: string;
	/** SDK の ShaderCompiler.exe。 */
	compiler: string;
	vsTarget: string;
	psTarget: string;
	/** 指定すると、そのファイルだけをコンパイルする(シェーダーのフォルダの中のものに限る)。 */
	only?: string[];
	/** 出力より新しいもの(#include しているファイルを含む)だけをコンパイルする(ビルド)。 */
	changedOnly?: boolean;
	/** 名前が VS/PS で終わらないファイルを「対象外」と出す。 */
	reportSkipped?: boolean;
}

export interface ShaderResult {
	/** 対象になったシェーダーの本数(変更なしを含む)。 */
	total: number;
	compiled: number;
	failed: number;
	skipped: number;
	upToDate: number;
	/** エラー・警告の行。元のファイルのパスに読み替えた `ファイル(行,列): error X3004: 内容` の形。 */
	diagnostics: string[];
}

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

/** シェーダーのフォルダにあるシェーダー(.hlsl・.fx)。フォルダが無ければ空。 */
export function listShaderSources(srcDir: string): string[] {
	return listFiles(srcDir, SOURCE_EXTS);
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

function mtime(file: string): number {
	try {
		return fs.statSync(file).mtimeMs;
	} catch {
		return 0;
	}
}

/** file と、そこから #include でたどれるファイルの、いちばん新しい更新時刻。 */
function newestWithIncludes(file: string, srcDir: string, seen = new Set<string>()): number {
	const key = path.resolve(file).toLowerCase();
	if (seen.has(key)) {
		return 0;
	}
	seen.add(key);
	let newest = mtime(file);
	let text: string;
	try {
		text = fs.readFileSync(file, 'utf8');
	} catch {
		return newest;
	}
	for (const m of text.matchAll(INCLUDE_LINE)) {
		// ShaderCompiler と同じく、そのファイルのフォルダ → シェーダーのフォルダの順に探す
		const found = [path.dirname(file), srcDir].map((d) => path.resolve(d, m[1])).find((p) => fs.existsSync(p));
		if (found) {
			newest = Math.max(newest, newestWithIncludes(found, srcDir, seen));
		}
	}
	return newest;
}

/** ShaderCompiler の出力を、一時フォルダの写しではなく元のファイルを指す形に直す。 */
function mapToOriginal(line: string, tmp: string, srcDir: string): string {
	let s = line;
	if (s.toLowerCase().startsWith(tmp.toLowerCase())) {
		s = srcDir + s.slice(tmp.length);
	}
	// 列が範囲(3,12-20)のときは先頭だけにする(ビルドの赤線が読む形。$msCompile と同じ)
	return s.replace(/^(.*?\(\d+,\d+)-\d+\)/, '$1)');
}

const DIAG_LINE = /^\S.*\(\d+(?:,\d+)?\)\s*:\s+(?:error|warning)\s+\w+\d+\s*:/i;
const ERROR_LINE = /:\s+error\s+X\d+\s*:/i;

/**
 * シェーダーをコンパイルする。
 * ShaderCompiler は CP932 のソースしか読めないので、一時フォルダに変換して渡す。
 * ShaderCompiler はエラーでも終了コード 0 で、失敗したときは既存の出力に触らない。そこで一時フォルダに出力させ、
 * 「出力ができた」かつ「エラーの行が無い」ときだけ成功として出力先に置く(DESIGN.md 9 章)。
 */
export async function compileShaderSet(job: ShaderJob, log: (line: string) => void): Promise<ShaderResult> {
	const result: ShaderResult = { total: 0, compiled: 0, failed: 0, skipped: 0, upToDate: 0, diagnostics: [] };
	let sources = listShaderSources(job.srcDir);
	if (job.only) {
		const wanted = new Set(job.only.map((f) => path.resolve(f).toLowerCase()));
		sources = sources.filter((f) => wanted.has(path.resolve(f).toLowerCase()));
	}

	// 種類と出力先を決め、ビルドなら変わったものだけに絞る
	const targets: { file: string; kind: ShaderKind; outFile: string }[] = [];
	for (const file of sources) {
		const kind = classify(file);
		if (!kind) {
			result.skipped++;
			if (job.reportSkipped) {
				log(`--  ${path.relative(job.srcDir, file)}: 名前の末尾が VS でも PS でもないので飛ばしました`);
			}
			continue;
		}
		result.total++;
		const outFile = path.join(job.outDir, path.basename(file, path.extname(file)) + (kind === 'vertex' ? '.vso' : '.pso'));
		if (job.changedOnly && fs.existsSync(outFile) && newestWithIncludes(file, job.srcDir) <= mtime(outFile)) {
			result.upToDate++;
			continue;
		}
		targets.push({ file, kind, outFile });
	}
	if (targets.length === 0) {
		return result;
	}
	log(`[DxLib] シェーダーをコンパイルします (${targets.length} 本) → ${path.relative(job.baseDir, job.outDir) || job.outDir}`);

	// 1. 一時フォルダへ CP932 で書き出す(include も一緒に)
	const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'dxlib-shader-'));
	const tmpOut = fs.mkdtempSync(path.join(os.tmpdir(), 'dxlib-shader-out-'));
	try {
		// CP932 に無い文字のあるファイルは写さない。コンパイルするシェーダーならここでエラーにし、
		// #include されるファイルなら、読んだシェーダーが「開けない」エラーになる
		const badFiles = new Map<string, { line: number; ch: string }>();
		for (const file of listFiles(job.srcDir, COPY_EXTS)) {
			const rel = path.relative(job.srcDir, file);
			const { bytes, badChar } = toCp932(fs.readFileSync(file, 'utf8'));
			if (badChar) {
				badFiles.set(path.resolve(file).toLowerCase(), badChar);
				continue;
			}
			const dst = path.join(tmp, rel);
			fs.mkdirSync(path.dirname(dst), { recursive: true });
			fs.writeFileSync(dst, bytes);
		}

		// 2. 1 本ずつコンパイル
		fs.mkdirSync(job.outDir, { recursive: true });
		for (const { file, kind, outFile } of targets) {
			const rel = path.relative(job.srcDir, file);
			const target = kind === 'vertex' ? job.vsTarget : job.psTarget;
			const bad = badFiles.get(path.resolve(file).toLowerCase());
			if (bad) {
				const diag = `${file}(${bad.line},1): error DX0001: CP932 で表せない文字「${bad.ch}」があります。取り除いてください。`;
				result.failed++;
				result.diagnostics.push(diag);
				log(`NG  ${rel} (${target})`);
				log(`      ${diag}`);
				continue;
			}
			const tmpSrc = path.join(tmp, rel);
			const tmpDst = path.join(tmpOut, path.basename(outFile));
			fs.rmSync(tmpDst, { force: true });
			const r = await run(job.compiler, [`/T${target}`, `/Fo${tmpDst}`, tmpSrc], path.dirname(tmpSrc));
			const lines = iconv
				.decode(Buffer.from(r.stdout + r.stderr, 'binary'), 'shift_jis')
				.split(/\r?\n/)
				.map((l) => mapToOriginal(l.trimEnd(), tmp, job.srcDir));
			const diags = lines.filter((l) => DIAG_LINE.test(l));
			result.diagnostics.push(...diags);
			if (r.code !== 0 || !fs.existsSync(tmpDst) || diags.some((l) => ERROR_LINE.test(l))) {
				result.failed++;
				log(`NG  ${rel} (${target})`);
				// エラーの行が取れないとき(ShaderCompiler が起動しないなど)は、出力をそのまま見せる
				const shown = diags.length > 0 ? diags : lines.filter((l) => l.trim());
				for (const l of shown) {
					log(`      ${l}`);
				}
			} else {
				fs.copyFileSync(tmpDst, outFile);
				result.compiled++;
				log(`OK  ${rel} (${target}) → ${path.relative(job.baseDir, outFile)}`);
				for (const l of diags) {
					log(`      ${l}`);
				}
			}
		}
	} finally {
		fs.rmSync(tmp, { recursive: true, force: true });
		fs.rmSync(tmpOut, { recursive: true, force: true });
	}
	return result;
}
