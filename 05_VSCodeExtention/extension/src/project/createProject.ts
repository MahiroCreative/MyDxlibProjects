import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import * as vscode from 'vscode';
import { getConfig, SDK_FIX_HINT, sdkProblem } from '../env/environment';
import { copyProjectTree, isEmptyDir, writeText } from '../util/fsx';
import { ensureProjectFiles } from '../build/vcxproj';
import { addRecentTemplate, findTemplate, templateSourceDir } from './templates';

export const PROJECT_NAME_RE = /^[A-Za-z_][A-Za-z0-9_]*$/;
export const PLACEHOLDER = '__PROJECT_NAME__';
export const CONFIG_PROVIDER_ID = 'mahirocreative.dxlib-devenv';

/**
 * プロジェクトに書く .clang-format(DESIGN.md 10 章)。C++ と HLSL で共通。
 * タブは字下げだけに使い、揃えは空白にする(タブと空白が混ざらない)。
 * HLSL のセマンティクスの `:` は、clang-format にはビットフィールドに見えるので AlignConsecutiveBitFields で揃う。
 * `public:` などは class と同じ位置(Microsoft の既定 -2 だと空白 2 個の字下げになり、タブと混ざる)。
 * 同梱のテンプレートと「ファイルを追加」の雛形は、この設定で整形しても変わらない形で書く。
 */
export const CLANG_FORMAT = [
	'BasedOnStyle: Microsoft',
	'UseTab: ForIndentation',
	'IndentWidth: 4',
	'TabWidth: 4',
	'BreakBeforeBraces: Allman',
	'ColumnLimit: 0',
	'AllowShortFunctionsOnASingleLine: Empty',
	'AllowShortIfStatementsOnASingleLine: WithoutElse',
	'PointerAlignment: Left',
	'SortIncludes: false',
	'NamespaceIndentation: All',
	'AlignConsecutiveBitFields: Consecutive',
	'AccessModifierOffset: -4',
	'',
].join('\n');

/** 当初の版が書いていた .clang-format(UseTab: Always)。 */
const CLANG_FORMAT_V1 = [
	'BasedOnStyle: Microsoft',
	'UseTab: Always',
	'IndentWidth: 4',
	'TabWidth: 4',
	'BreakBeforeBraces: Allman',
	'ColumnLimit: 0',
	'AllowShortFunctionsOnASingleLine: Empty',
	'AllowShortIfStatementsOnASingleLine: WithoutElse',
	'PointerAlignment: Left',
	'SortIncludes: false',
	'NamespaceIndentation: All',
	'',
].join('\n');

/** 2026-09-24 の版が書いていた .clang-format(AccessModifierOffset なし)。 */
const CLANG_FORMAT_V2 = CLANG_FORMAT.replace('AccessModifierOffset: -4\n', '');

/** 以前にこの拡張が書いた .clang-format。どれかと完全に同じなら、開いたときに CLANG_FORMAT に書き換える(手で直したものは触らない)。 */
export const OLD_CLANG_FORMATS = [CLANG_FORMAT_V1, CLANG_FORMAT_V2];

export interface CreateProjectArgs {
	name: string;
	location: string;
	templateId: string;
}

export function validateProjectName(name: string): string | undefined {
	if (!name) {
		return 'プロジェクト名を入力してください。';
	}
	if (!PROJECT_NAME_RE.test(name)) {
		return 'プロジェクト名は英数字とアンダースコアだけで、先頭は英字か _ にしてください。';
	}
	return undefined;
}

/** テンプレートからプロジェクトを作り、作成したフォルダのパスを返す。 */
export async function createProject(context: vscode.ExtensionContext, args: CreateProjectArgs): Promise<string> {
	// SDK が正しくない間はプロジェクトを作成できない(作っても、ビルドできないため)。
	const problem = sdkProblem();
	if (problem) {
		throw new Error(`${problem}プロジェクトを作成できません。${SDK_FIX_HINT}`);
	}
	const nameError = validateProjectName(args.name);
	if (nameError) {
		throw new Error(nameError);
	}
	if (!args.location || !fs.existsSync(args.location)) {
		throw new Error('作成先のフォルダが存在しません。');
	}
	const template = findTemplate(context, args.templateId);
	if (!template) {
		throw new Error('テンプレートが見つかりません(テンプレートファイルが移動・削除されたか、テンプレートのファイルではありません)。');
	}
	const dest = path.join(args.location, args.name);
	if (!isEmptyDir(dest)) {
		throw new Error(`フォルダが既に存在します: ${dest}`);
	}

	// テンプレートファイル(.dxtemplate)は一時フォルダに展開してから、同梱テンプレートと同じ手順でコピー・置換する
	const source = templateSourceDir(template);
	try {
		const replaceAll = (s: string): string => s.split(PLACEHOLDER).join(args.name);
		copyProjectTree(source.dir, dest, {
			renameEntry: replaceAll,
			transformText: (text) => replaceAll(text),
		});
	} finally {
		source.cleanup();
	}
	if (!template.builtin) {
		addRecentTemplate(context, template.source);
	}
	if (!fs.existsSync(path.join(dest, 'src'))) {
		fs.mkdirSync(path.join(dest, 'src'), { recursive: true });
	}
	writeProjectFiles(dest, args.name);
	// 次に作るときの作成先の初期値にする
	recordLastLocation(context, args.location);
	return dest;
}

/**
 * 前回の作成先の記録ファイル(拡張機能の保存フォルダ。PC ごと)。
 * globalState は VSCode が後でまとめてディスクに書くので、すぐに終了すると残らないことがある。
 * その場でファイルに書き、窓や再起動をまたいで確実に残す。
 */
function lastLocationFile(context: vscode.ExtensionContext): string {
	return path.join(context.globalStorageUri.fsPath, 'last-create-location.txt');
}

function recordLastLocation(context: vscode.ExtensionContext, location: string): void {
	try {
		fs.mkdirSync(context.globalStorageUri.fsPath, { recursive: true });
		fs.writeFileSync(lastLocationFile(context), location, 'utf8');
	} catch {
		// 記録できなくても作成自体は成功している。初期値が従来どおりになるだけ。
	}
}

/**
 * 作成先の初期値。前回プロジェクトを作った場所。記録が無いかフォルダが無くなっていたら、
 * 開いているプロジェクトの親フォルダ、それも無ければホームフォルダ。
 */
export function defaultCreateLocation(context: vscode.ExtensionContext): string {
	let last: string | undefined;
	try {
		last = fs.readFileSync(lastLocationFile(context), 'utf8').trim();
	} catch {
		last = undefined;
	}
	if (last && fs.existsSync(last)) {
		return last;
	}
	const folder = vscode.workspace.workspaceFolders?.[0];
	return folder ? path.dirname(folder.uri.fsPath) : os.homedir();
}

/** Visual Studio で作ったプロジェクトを使えるようにするとき(DESIGN.md 6.1 章)。exe と作業フォルダは MSBuild に聞いた値。 */
export interface VsProjectFiles {
	vcxproj: string;
	platform: string;
	debug: { exe: string; cwd: string };
	release: { exe: string; cwd: string };
}

/** プロジェクトのフォルダの中なら ${workspaceFolder} からの形にする(フォルダを動かしても使えるように)。 */
function inWorkspace(projectDir: string, p: string): string {
	const rel = path.relative(projectDir, p);
	return rel && !rel.startsWith('..') && !path.isAbsolute(rel) ? `\${workspaceFolder}/${rel.replace(/\\/g, '/')}` : p;
}

/**
 * .vscode 一式、.clang-format、.gitignore を書く。既存プロジェクトへの追加にも使う。
 * vs を渡したとき(Visual Studio で作ったプロジェクト)は、生徒のコードの書き方を変えないよう .clang-format・.gitignore を書かず、
 * 保存時の整形も切り、この拡張の .vcxproj も作らない(DESIGN.md 6.1 章)。
 */
export function writeProjectFiles(projectDir: string, projectName: string, vs?: VsProjectFiles): void {
	const cppStandard = getConfig<string>('build.cppStandard', 'c++20');

	const tasks = {
		version: '2.0.0',
		tasks: [
			{
				type: 'dxlib',
				config: 'debug',
				label: 'DxLib: Debug ビルド',
				group: { kind: 'build', isDefault: true },
				problemMatcher: [], // ビルドエラーの赤線は拡張が付ける(DESIGN.md 6 章)
			},
			{
				type: 'dxlib',
				config: 'release',
				label: 'DxLib: Release ビルド',
				group: 'build',
				problemMatcher: [], // ビルドエラーの赤線は拡張が付ける(DESIGN.md 6 章)
			},
		],
	};

	const launch = {
		version: '0.2.0',
		configurations: [
			{
				name: 'DxLib: デバッグ実行 (Debug)',
				type: 'cppvsdbg',
				request: 'launch',
				program: vs ? inWorkspace(projectDir, vs.debug.exe) : `\${workspaceFolder}/build/Debug/${projectName}.exe`,
				args: [],
				stopAtEntry: false,
				cwd: vs ? inWorkspace(projectDir, vs.debug.cwd) : '${workspaceFolder}',
				environment: [],
				console: 'internalConsole',
				preLaunchTask: 'DxLib: Debug ビルド',
			},
			{
				name: 'DxLib: 実行 (Release)',
				type: 'cppvsdbg',
				request: 'launch',
				program: vs ? inWorkspace(projectDir, vs.release.exe) : `\${workspaceFolder}/build/Release/${projectName}.exe`,
				args: [],
				stopAtEntry: false,
				cwd: vs ? inWorkspace(projectDir, vs.release.cwd) : '${workspaceFolder}',
				environment: [],
				console: 'internalConsole',
				preLaunchTask: 'DxLib: Release ビルド',
			},
		],
	};

	const cppProperties = {
		version: 4,
		configurations: [
			{
				name: 'DxLib',
				configurationProvider: CONFIG_PROVIDER_ID,
				intelliSenseMode: vs && vs.platform !== 'x64' ? 'windows-msvc-x86' : 'windows-msvc-x64',
				cStandard: 'c17',
				cppStandard: cppStandard === 'c++latest' ? 'c++23' : cppStandard,
			},
		],
	};

	const settings: Record<string, unknown> = {
		// BOM 付き UTF-8。BOM が無いと cl.exe が CP932 として読み、日本語コメントで壊れる。
		'files.encoding': 'utf8bom',
		'files.eol': '\n',
		'editor.formatOnSave': true,
		'editor.insertSpaces': false,
		'editor.tabSize': 4,
		'[cpp]': { 'editor.defaultFormatter': 'ms-vscode.cpptools' },
		'[c]': { 'editor.defaultFormatter': 'ms-vscode.cpptools' },
		'[hlsl]': { 'editor.defaultFormatter': 'mahirocreative.dxlib-devenv' },
		'files.associations': { '*.fx': 'hlsl', '*.hlsli': 'hlsl' },
		'C_Cpp.default.configurationProvider': CONFIG_PROVIDER_ID,
		// C/C++ 拡張がエディタ右上に出す ▶(C/C++ ファイルの実行)を消す。DxLib 拡張のビルド・実行と紛らわしい(DESIGN.md 3.2 章)
		'C_Cpp.debugShortcut': false,
	};
	if (vs) {
		// その .vcxproj をそのまま MSBuild でビルドする。生徒のコードの書き方は変えない(DESIGN.md 6.1 章)
		settings['dxlib.vsProject'] = vs.vcxproj;
		settings['dxlib.vsPlatform'] = vs.platform;
		settings['editor.formatOnSave'] = false;
	}

	// .cpp を開くと VSCode が「C/C++ Extension Pack」を勧めてくるのを止める。
	// Pack には CMake Tools などが入るが、このツールは cl.exe を直接呼ぶので要らない。
	const extensions = {
		unwantedRecommendations: ['ms-vscode.cpptools-extension-pack'],
	};

	// dxlib.props は PC ごとの SDK の場所なので入れない(DESIGN.md 6 章)
	const gitignore = ['build/', 'Log.txt', '*.pdb', '*.ilk', '*.obj', '.vs/', '*.vcxproj.user', 'dxlib.props', ''].join('\n');

	writeText(path.join(projectDir, '.vscode', 'tasks.json'), JSON.stringify(tasks, null, '\t') + '\n');
	writeText(path.join(projectDir, '.vscode', 'launch.json'), JSON.stringify(launch, null, '\t') + '\n');
	writeText(path.join(projectDir, '.vscode', 'c_cpp_properties.json'), JSON.stringify(cppProperties, null, '\t') + '\n');
	writeText(path.join(projectDir, '.vscode', 'settings.json'), JSON.stringify(settings, null, '\t') + '\n');
	writeText(path.join(projectDir, '.vscode', 'extensions.json'), JSON.stringify(extensions, null, '\t') + '\n');
	if (vs) {
		return;
	}
	writeText(path.join(projectDir, '.clang-format'), CLANG_FORMAT);
	if (!fs.existsSync(path.join(projectDir, '.gitignore'))) {
		writeText(path.join(projectDir, '.gitignore'), gitignore);
	}
	// MSBuild / Visual Studio 用のファイル(.vcxproj・.sln・dxlib.props。DESIGN.md 6 章)
	ensureProjectFiles(projectDir, projectName, getConfig<string>('sdkPath', ''), cppStandard);
}
