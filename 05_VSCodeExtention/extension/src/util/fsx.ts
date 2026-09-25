import * as fs from 'fs';
import * as path from 'path';

/** テンプレートやプロジェクトのコピーで除外するフォルダ名。 */
const EXCLUDED_DIRS = new Set(['.vscode', '.git', '.vs', 'build', 'bin', 'obj', 'x64', 'x86', 'Debug', 'Release', 'node_modules']);

/** コピーで除外するファイル名。 */
const EXCLUDED_FILES = new Set(['Log.txt', 'template.json', 'dxlib.props']);

/** コピーで除外するファイルの拡張子。MSBuild / Visual Studio 用のファイルは、作るときにその名前で作り直す(DESIGN.md 6 章)。 */
const EXCLUDED_FILE_RE = /\.(sln|vcxproj|vcxproj\.filters|vcxproj\.user)$/i;

/** 内容の置換対象にするテキストファイルの拡張子。 */
const TEXT_EXTS = new Set(['.cpp', '.c', '.h', '.hpp', '.inl', '.hlsl', '.hlsli', '.fx', '.fxh', '.txt', '.md', '.json', '.ini', '.csv', '.xml']);
const TEXT_NAMES = new Set(['.clang-format', '.gitignore', '.editorconfig']);

export function isTextFile(name: string): boolean {
	return TEXT_EXTS.has(path.extname(name).toLowerCase()) || TEXT_NAMES.has(name);
}

/** cl.exe / ShaderCompiler に渡すソース。BOM 付き UTF-8 で保存する(BOM が無いと CP932 として読まれる)。 */
const BOM_EXTS = new Set(['.cpp', '.c', '.h', '.hpp', '.inl', '.hlsl', '.hlsli', '.fx', '.fxh']);

export function needsBom(name: string): boolean {
	return BOM_EXTS.has(path.extname(name).toLowerCase());
}

export function withBom(text: string, bom: boolean): string {
	const body = text.replace(/^\uFEFF/, '');
	return bom ? '\uFEFF' + body : body;
}

/** DxLib SDK のコピー(DxLib.h を含むフォルダ)かどうか。テンプレートには含めない。 */
export function isSdkCopy(dir: string): boolean {
	return fs.existsSync(path.join(dir, 'DxLib.h')) && fs.existsSync(path.join(dir, 'DxDataTypeWin.h'));
}

export interface CopyOptions {
	/** ファイル名・フォルダ名の変換。 */
	renameEntry?: (name: string) => string;
	/** テキストファイルの内容の変換。 */
	transformText?: (text: string, relPath: string) => string;
}

/**
 * フォルダを再帰的にコピーする。ビルド成果物・.vscode・SDK コピー・Log.txt は除外する。
 * 戻り値はコピーしたファイル数。
 */
export function copyProjectTree(src: string, dst: string, options: CopyOptions = {}): number {
	let count = 0;
	const walk = (from: string, to: string, rel: string): void => {
		fs.mkdirSync(to, { recursive: true });
		for (const entry of fs.readdirSync(from, { withFileTypes: true })) {
			const srcPath = path.join(from, entry.name);
			const outName = options.renameEntry ? options.renameEntry(entry.name) : entry.name;
			const dstPath = path.join(to, outName);
			const relPath = rel ? `${rel}/${outName}` : outName;
			if (entry.isDirectory()) {
				if (EXCLUDED_DIRS.has(entry.name) || isSdkCopy(srcPath)) {
					continue;
				}
				walk(srcPath, dstPath, relPath);
			} else if (entry.isFile()) {
				if (EXCLUDED_FILES.has(entry.name) || EXCLUDED_FILE_RE.test(entry.name)) {
					continue;
				}
				if (options.transformText && isTextFile(entry.name)) {
					const text = fs.readFileSync(srcPath, 'utf8');
					fs.writeFileSync(dstPath, withBom(options.transformText(text, relPath), needsBom(outName)), 'utf8');
				} else {
					fs.copyFileSync(srcPath, dstPath);
				}
				count++;
			}
		}
	};
	walk(src, dst, '');
	return count;
}

/** UTF-8 でテキストを書く。C/C++/HLSL のソースは BOM 付き、それ以外は BOM なし。親フォルダが無ければ作る。 */
export function writeText(file: string, text: string): void {
	fs.mkdirSync(path.dirname(file), { recursive: true });
	fs.writeFileSync(file, withBom(text, needsBom(file)), 'utf8');
}

export function isEmptyDir(dir: string): boolean {
	return !fs.existsSync(dir) || fs.readdirSync(dir).length === 0;
}
