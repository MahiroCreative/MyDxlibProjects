import * as fs from 'fs';
import * as iconv from 'iconv-lite';
import * as path from 'path';
import * as vscode from 'vscode';
import { run } from '../util/exec';
import { isOwnVcxprojText } from './vcxproj';

/**
 * Visual Studio で作ったプロジェクト(DESIGN.md 6.1 章)。
 * その .vcxproj をそのまま MSBuild でビルドし、設定は MSBuild に聞く(Visual Studio と同じ解釈にする)。
 * .vcxproj は、VSCode でファイルを足したときに一覧へ書き足す以外は触らない。
 */

/** フォルダの直下にある、この拡張が作ったものではない .vcxproj(名前順で最初)。無ければ undefined。 */
export function findVsProject(dir: string): string | undefined {
	let names: string[];
	try {
		names = fs.readdirSync(dir).filter((n) => n.toLowerCase().endsWith('.vcxproj')).sort((a, b) => a.localeCompare(b));
	} catch {
		return undefined;
	}
	for (const n of names) {
		try {
			if (!isOwnVcxprojText(fs.readFileSync(path.join(dir, n), 'utf8'))) {
				return n;
			}
		} catch {
			// 読めないものは飛ばす
		}
	}
	return undefined;
}

/** ビルドに使うプラットフォーム。x64 の構成があれば x64、無ければ Win32。 */
export function pickPlatform(vcxprojText: string): 'x64' | 'Win32' {
	return /Include="Debug\|x64"/i.test(vcxprojText) ? 'x64' : 'Win32';
}

export interface VsProjectInfo {
	targetPath: string;
	workingDir: string;
	includeDirs: string[];
	defines: string[];
	languageStandard: string;
}

/** MSBuild に、構成ごとの出力先・作業フォルダ・インクルードの場所・定義・C++ 規格を聞く(ビルドはしない)。 */
export async function queryVsProject(msbuild: string, vcxproj: string, config: 'Debug' | 'Release', platform: string): Promise<VsProjectInfo | undefined> {
	const r = await run(
		msbuild,
		[vcxproj, '-nologo', `-p:Configuration=${config}`, `-p:Platform=${platform}`, '-getProperty:TargetPath,LocalDebuggerWorkingDirectory,ProjectDir', '-getItem:ClCompile'],
		path.dirname(vcxproj),
	);
	let json: { Properties?: Record<string, string>; Items?: { ClCompile?: Array<Record<string, string>> } };
	try {
		json = JSON.parse(r.stdout.slice(r.stdout.indexOf('{')));
	} catch {
		return undefined;
	}
	const props = json.Properties ?? {};
	const item = json.Items?.ClCompile?.[0] ?? {};
	const split = (s: string | undefined): string[] =>
		(s ?? '')
			.split(';')
			.map((x) => x.trim())
			.filter((x) => x && !x.startsWith('%('));
	const projectDir = props.ProjectDir || path.dirname(vcxproj);
	return {
		targetPath: props.TargetPath ?? '',
		workingDir: props.LocalDebuggerWorkingDirectory || projectDir,
		includeDirs: split(item.AdditionalIncludeDirectories).map((d) => (path.isAbsolute(d) ? d : path.resolve(projectDir, d))),
		defines: split(item.PreprocessorDefinitions),
		languageStandard: item.LanguageStandard ?? '',
	};
}

export type SourceEncoding = 'ascii' | 'utf8bom' | 'utf8' | 'sjis';

export function classifyEncoding(buf: Buffer): SourceEncoding {
	if (buf[0] === 0xef && buf[1] === 0xbb && buf[2] === 0xbf) {
		return 'utf8bom';
	}
	if (!buf.some((b) => b >= 0x80)) {
		return 'ascii';
	}
	// 正しい UTF-8 として読めるなら UTF-8、読めなければ Shift-JIS(VS 2022 以前の既定)とみなす
	return Buffer.from(buf.toString('utf8'), 'utf8').equals(buf) ? 'utf8' : 'sjis';
}

/** ビルド成果物などを除いた、プロジェクトのフォルダの下の .cpp・.h・.hpp。 */
export function projectSources(dir: string): string[] {
	const skip = new Set(['.vs', '.vscode', '.git', 'x64', 'x86', 'Win32', 'Debug', 'Release', 'build', 'bin', 'obj', 'node_modules']);
	const out: string[] = [];
	const walk = (d: string): void => {
		for (const e of fs.readdirSync(d, { withFileTypes: true })) {
			const full = path.join(d, e.name);
			if (e.isDirectory()) {
				if (!skip.has(e.name)) {
					walk(full);
				}
			} else if (/\.(cpp|h|hpp)$/i.test(e.name)) {
				out.push(full);
			}
		}
	};
	walk(dir);
	return out;
}

/** BOM 付き UTF-8 にそろえる(Shift-JIS は CP932 として読んで変換する)。変えたファイルを返す。 */
export function convertToUtf8Bom(files: string[]): string[] {
	const changed: string[] = [];
	for (const f of files) {
		const buf = fs.readFileSync(f);
		const kind = classifyEncoding(buf);
		if (kind === 'utf8' || kind === 'sjis') {
			const text = kind === 'utf8' ? buf.toString('utf8') : iconv.decode(buf, 'cp932');
			fs.writeFileSync(f, '\uFEFF' + text, 'utf8');
			changed.push(f);
		}
	}
	return changed;
}

/** 読み書きしても BOM と改行の形(CRLF/LF)を保つ。 */
function readKeep(file: string): { text: string; bom: boolean; crlf: boolean } {
	const raw = fs.readFileSync(file, 'utf8');
	const bom = raw.startsWith('\uFEFF');
	const text = bom ? raw.slice(1) : raw;
	return { text: text.replace(/\r\n/g, '\n'), bom, crlf: text.includes('\r\n') };
}

function writeKeep(file: string, text: string, bom: boolean, crlf: boolean): void {
	fs.writeFileSync(file, (bom ? '\uFEFF' : '') + (crlf ? text.replace(/\n/g, '\r\n') : text), 'utf8');
}

function xmlAttr(s: string): string {
	return s.replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;');
}

/**
 * VSCode で足したファイルを .vcxproj(ClCompile / ClInclude)と .filters に書き足す。
 * 同じ種類の項目があればその並びの後ろ、無ければ新しい ItemGroup を Microsoft.Cpp.targets の Import の前に入れる。
 * .filters では、同じ種類のファイルが入っているフィルターに入れる(無ければフィルターなし)。
 */
export function addToVsProject(vcxproj: string, files: string[]): void {
	const projectDir = path.dirname(vcxproj);
	const entries = files.map((f) => ({ rel: path.relative(projectDir, f), tag: /\.(cpp|c|cc|cxx)$/i.test(f) ? 'ClCompile' : 'ClInclude' }));

	const insert = (text: string, tag: string, lines: string[]): string => {
		const re = new RegExp(`^([ \\t]*)<${tag} Include="[^"]*"\\s*(/>|>[\\s\\S]*?</${tag}>)[ \\t]*$`, 'gm');
		let last: RegExpExecArray | undefined;
		for (let m = re.exec(text); m; m = re.exec(text)) {
			last = m;
		}
		if (last) {
			const at = last.index + last[0].length;
			const indent = last[1];
			return text.slice(0, at) + lines.map((l) => '\n' + indent + l).join('') + text.slice(at);
		}
		const importRe = /^([ \t]*)<Import Project="\$\(VCTargetsPath\)\\Microsoft\.Cpp\.targets"\s*\/>/m;
		const im = importRe.exec(text);
		const indent = im ? im[1] : '  ';
		const group = `${indent}<ItemGroup>\n${lines.map((l) => `${indent}  ${l}`).join('\n')}\n${indent}</ItemGroup>\n`;
		if (im) {
			return text.slice(0, im.index) + group + text.slice(im.index);
		}
		return text.replace(/<\/Project>\s*$/, `${group}</Project>\n`);
	};

	const v = readKeep(vcxproj);
	let vt = v.text;
	for (const tag of ['ClCompile', 'ClInclude']) {
		const mine = entries.filter((e) => e.tag === tag && !vt.includes(`Include="${xmlAttr(e.rel)}"`));
		if (mine.length > 0) {
			vt = insert(vt, tag, mine.map((e) => `<${tag} Include="${xmlAttr(e.rel)}" />`));
		}
	}
	writeKeep(vcxproj, vt, v.bom, v.crlf);

	const filters = `${vcxproj}.filters`;
	if (!fs.existsSync(filters)) {
		return;
	}
	const f = readKeep(filters);
	let ft = f.text;
	for (const tag of ['ClCompile', 'ClInclude']) {
		const mine = entries.filter((e) => e.tag === tag && !ft.includes(`Include="${xmlAttr(e.rel)}"`));
		if (mine.length === 0) {
			continue;
		}
		const filter = new RegExp(`<${tag} Include="[^"]*">\\s*<Filter>([^<]*)</Filter>`).exec(ft)?.[1];
		const lines = mine.flatMap((e) =>
			filter ? [`<${tag} Include="${xmlAttr(e.rel)}">`, `  <Filter>${filter}</Filter>`, `</${tag}>`] : [`<${tag} Include="${xmlAttr(e.rel)}" />`],
		);
		ft = insert(ft, tag, lines);
	}
	writeKeep(filters, ft, f.bom, f.crlf);
}

/** 今開いているフォルダが、使えるようにした Visual Studio のプロジェクトなら、その .vcxproj(絶対パス)。 */
export function vsProjectOf(folder: vscode.WorkspaceFolder | undefined): { vcxproj: string; platform: string } | undefined {
	if (!folder) {
		return undefined;
	}
	const cfg = vscode.workspace.getConfiguration('dxlib', folder.uri);
	const name = cfg.get<string>('vsProject', '');
	if (!name) {
		return undefined;
	}
	return { vcxproj: path.join(folder.uri.fsPath, name), platform: cfg.get<string>('vsPlatform', 'x64') || 'x64' };
}
