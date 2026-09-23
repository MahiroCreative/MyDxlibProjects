import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { getConfig, SDK_FIX_HINT, sdkProblem } from '../env/environment';
import { copyProjectTree, isEmptyDir, writeText } from '../util/fsx';
import { findTemplate } from './templates';

export const PROJECT_NAME_RE = /^[A-Za-z_][A-Za-z0-9_]*$/;
export const PLACEHOLDER = '__PROJECT_NAME__';
export const CONFIG_PROVIDER_ID = 'mahirocreative.dxlib-devenv';

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
	const template = findTemplate(context, getConfig<string>('templatesPath', ''), args.templateId);
	if (!template) {
		throw new Error('テンプレートが見つかりません。');
	}
	const dest = path.join(args.location, args.name);
	if (!isEmptyDir(dest)) {
		throw new Error(`フォルダが既に存在します: ${dest}`);
	}

	const replaceAll = (s: string): string => s.split(PLACEHOLDER).join(args.name);
	copyProjectTree(template.dir, dest, {
		renameEntry: replaceAll,
		transformText: (text) => replaceAll(text),
	});
	if (!fs.existsSync(path.join(dest, 'src'))) {
		fs.mkdirSync(path.join(dest, 'src'), { recursive: true });
	}
	writeProjectFiles(dest, args.name);
	return dest;
}

/** .vscode 一式、.clang-format、.gitignore を書く。既存プロジェクトへの追加にも使う。 */
export function writeProjectFiles(projectDir: string, projectName: string): void {
	const cppStandard = getConfig<string>('build.cppStandard', 'c++20');

	const tasks = {
		version: '2.0.0',
		tasks: [
			{
				type: 'dxlib',
				config: 'debug',
				label: 'DxLib: Debug ビルド',
				group: { kind: 'build', isDefault: true },
				problemMatcher: ['$msCompile'],
			},
			{
				type: 'dxlib',
				config: 'release',
				label: 'DxLib: Release ビルド',
				group: 'build',
				problemMatcher: ['$msCompile'],
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
				program: `\${workspaceFolder}/build/Debug/${projectName}.exe`,
				args: [],
				stopAtEntry: false,
				cwd: '${workspaceFolder}',
				environment: [],
				externalConsole: false,
				preLaunchTask: 'DxLib: Debug ビルド',
			},
			{
				name: 'DxLib: 実行 (Release)',
				type: 'cppvsdbg',
				request: 'launch',
				program: `\${workspaceFolder}/build/Release/${projectName}.exe`,
				args: [],
				stopAtEntry: false,
				cwd: '${workspaceFolder}',
				environment: [],
				externalConsole: false,
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
				intelliSenseMode: 'windows-msvc-x64',
				cStandard: 'c17',
				cppStandard: cppStandard === 'c++latest' ? 'c++23' : cppStandard,
			},
		],
	};

	const settings = {
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
	};

	const clangFormat = [
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

	const gitignore = ['build/', 'Log.txt', '*.pdb', '*.ilk', '*.obj', '.vs/', ''].join('\n');

	writeText(path.join(projectDir, '.vscode', 'tasks.json'), JSON.stringify(tasks, null, '\t') + '\n');
	writeText(path.join(projectDir, '.vscode', 'launch.json'), JSON.stringify(launch, null, '\t') + '\n');
	writeText(path.join(projectDir, '.vscode', 'c_cpp_properties.json'), JSON.stringify(cppProperties, null, '\t') + '\n');
	writeText(path.join(projectDir, '.vscode', 'settings.json'), JSON.stringify(settings, null, '\t') + '\n');
	writeText(path.join(projectDir, '.clang-format'), clangFormat);
	if (!fs.existsSync(path.join(projectDir, '.gitignore'))) {
		writeText(path.join(projectDir, '.gitignore'), gitignore);
	}
}
