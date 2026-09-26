import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { projectExeName } from '../env/environment';
import { detectVisualStudio } from '../env/vswhere';
import { writeZip, ZipEntry } from '../util/zip';
import { runBuild } from './taskProvider';
import { queryVsProject, vsProjectOf } from './vsProject';

/**
 * 配布用にまとめる(DESIGN.md 6.2 章)。
 * Release でビルドし、exe と素材を dist\<名前>\ にプロジェクトと同じ並びでコピーして、zip も作る。
 * 素材は「入れないもの」以外すべて(生徒が素材のフォルダに好きな名前を付けても入るように)。
 */

const SKIP_DIRS = new Set(['src', '.vscode', '.vs', '.git', 'build', 'dist', 'x64', 'x86', 'win32', 'debug', 'release', 'node_modules']);
const SKIP_EXT = /\.(cpp|c|cc|cxx|h|hpp|hxx|inl|hlsl|hlsli|fx|fxh|vcxproj|filters|user|sln|slnx|pdb|ilk|obj|dxtemplate)$/i;
const SKIP_NAMES = new Set(['dxlib.props', '.clang-format', '.gitignore', '.gitattributes', 'template.json', 'log.txt']);

export interface PackageResult {
	ok: boolean;
	error?: string;
	/** dist\<名前>\ */
	dir?: string;
	zip?: string;
	/** 入れたファイル(dir からの相対パス)。 */
	files?: string[];
}

/** ビルドのタスクを実行して、終わるまで待つ(終了コードを返す)。 */
function waitBuild(execution: vscode.TaskExecution | undefined): Promise<number | undefined> {
	if (!execution) {
		return Promise.resolve(undefined);
	}
	return new Promise((resolve) => {
		const sub = vscode.tasks.onDidEndTaskProcess((e) => {
			if (e.execution === execution || e.execution.task.definition.type === 'dxlib') {
				sub.dispose();
				resolve(e.exitCode);
			}
		});
	});
}

/** 素材を集める(base からの相対パス)。 */
function collectAssets(base: string, extraSkipDirs: Set<string>): string[] {
	const out: string[] = [];
	const walk = (dir: string, rel: string, top: boolean): void => {
		for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
			const lower = e.name.toLowerCase();
			const relPath = rel ? path.join(rel, e.name) : e.name;
			if (e.isDirectory()) {
				// ビルドの成果物などは一番上の階層だけで除く(素材のフォルダの中の「data\Release」などは残す)
				if (top && (SKIP_DIRS.has(lower) || extraSkipDirs.has(lower))) {
					continue;
				}
				if (lower === '.vs' || lower === '.git' || lower === '.vscode') {
					continue;
				}
				walk(path.join(dir, e.name), relPath, false);
			} else if (e.isFile() && !SKIP_EXT.test(e.name) && !SKIP_NAMES.has(lower)) {
				out.push(relPath);
			}
		}
	};
	walk(base, '', true);
	return out;
}

export async function packageRelease(folder: vscode.WorkspaceFolder): Promise<PackageResult> {
	const proj = folder.uri.fsPath;
	const name = projectExeName(folder);

	// 1. Release でビルドする
	const code = await waitBuild(await runBuild('release'));
	if (code !== 0) {
		return { ok: false, error: 'Release ビルドに失敗したので、まとめられませんでした。ターミナルのエラーを直してから、もう一度押してください。' };
	}

	// 2. exe の場所と、素材の起点(実行したときの作業フォルダ)
	let exe = path.join(proj, 'build', 'Release', `${name}.exe`);
	let base = proj;
	const extraSkip = new Set<string>();
	const vsp = vsProjectOf(folder);
	if (vsp) {
		const vs = await detectVisualStudio();
		const info = vs.msbuild ? await queryVsProject(vs.msbuild, vsp.vcxproj, 'Release', vsp.platform) : undefined;
		if (!info || !info.targetPath) {
			return { ok: false, error: 'Visual Studio のプロジェクトの Release の出力先を MSBuild から読めませんでした。' };
		}
		exe = info.targetPath;
		base = info.workingDir;
		// Visual Studio の既定の中間ファイルのフォルダ(<プロジェクト名>\x64\Release など)
		extraSkip.add(path.basename(vsp.vcxproj, '.vcxproj').toLowerCase());
	}
	if (!fs.existsSync(exe)) {
		return { ok: false, error: `Release の exe が見つかりません: ${exe}` };
	}

	// 3. dist\<名前>\ を作り直してコピーする
	const distRoot = path.join(proj, 'dist');
	const dir = path.join(distRoot, name);
	const zip = path.join(distRoot, `${name}.zip`);
	fs.rmSync(dir, { recursive: true, force: true });
	fs.rmSync(zip, { force: true });
	fs.mkdirSync(dir, { recursive: true });
	const files: string[] = [];
	fs.copyFileSync(exe, path.join(dir, path.basename(exe)));
	files.push(path.basename(exe));
	// exe と同じフォルダの DLL(ほかのライブラリを使っているとき)
	for (const f of fs.readdirSync(path.dirname(exe))) {
		if (/\.dll$/i.test(f)) {
			fs.copyFileSync(path.join(path.dirname(exe), f), path.join(dir, f));
			files.push(f);
		}
	}
	for (const rel of collectAssets(base, extraSkip)) {
		const to = path.join(dir, rel);
		fs.mkdirSync(path.dirname(to), { recursive: true });
		fs.copyFileSync(path.join(base, rel), to);
		files.push(rel);
	}

	// 4. 同じ中身の zip(展開すると <名前> フォルダができる)
	const entries: ZipEntry[] = files.map((rel) => ({ name: `${name}/${rel.replace(/\\/g, '/')}`, data: fs.readFileSync(path.join(dir, rel)) }));
	fs.writeFileSync(zip, writeZip(entries));
	return { ok: true, dir, zip, files };
}
