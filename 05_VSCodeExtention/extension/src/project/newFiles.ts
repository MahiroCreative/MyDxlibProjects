import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { addToVsProject, vsProjectOf } from '../build/vsProject';
import { currentFolder } from '../env/environment';

/**
 * 「ファイルを追加」(DESIGN.md 3.2 章)。
 * エクスプローラーの「DxLib」欄のボタン・右クリックの「DxLib」メニュー・見出しのボタンから呼ばれる。
 * 名前は欄の中のフォームで聞く(画面上部の入力欄は生徒が UI と認識しにくいので使わない)。
 */

export type CppFileKind = 'cpp' | 'h' | 'class';

/** 欄のフォームからの送信(と検証)で、コマンドに渡す引数。folder は作る場所(src の下)。 */
export interface NewCppArgs {
	name: string;
	folder: string;
}

export interface NewFilesResult {
	ok: boolean;
	error?: string;
	files?: string[];
}

const NAME_RE = /^[A-Za-z_][A-Za-z0-9_]*$/;
const NAME_ERROR = '名前は英数字とアンダースコアだけで、先頭は英字か _ にしてください。';

export const KIND_LABEL: Record<CppFileKind, string> = {
	cpp: 'C++ ソース (.cpp)',
	h: 'ヘッダー (.h)',
	class: 'クラス (.h と .cpp)',
};

/** プロジェクトの src フォルダ。ビルドはこの下の .cpp だけを対象にする。 */
export function cppRootDir(project: vscode.WorkspaceFolder): string {
	// Visual Studio で作ったプロジェクトは、ソースがフォルダの直下にある(DESIGN.md 6.1 章)
	return vsProjectOf(project) ? project.uri.fsPath : path.join(project.uri.fsPath, 'src');
}

/** C++ のファイルを作る場所の一覧に入れないフォルダ(ビルド成果物など。Visual Studio で作ったプロジェクト用)。 */
const NOT_SOURCE_DIRS = new Set(['.vs', '.vscode', '.git', 'x64', 'x86', 'Win32', 'Debug', 'Release', 'build', 'bin', 'obj', 'node_modules', 'shaders']);

function isInsideOrSame(child: string, parent: string): boolean {
	const rel = path.relative(parent.toLowerCase(), child.toLowerCase());
	return rel === '' || (!rel.startsWith('..') && !path.isAbsolute(rel));
}

/** src とその下のすべてのフォルダ(右クリックの「DxLib」メニューの表示条件に使う)。 */
export function listCppFolders(project: vscode.WorkspaceFolder): string[] {
	const result: string[] = [];
	const walk = (dir: string): void => {
		result.push(dir);
		let entries: fs.Dirent[];
		try {
			entries = fs.readdirSync(dir, { withFileTypes: true });
		} catch {
			return;
		}
		for (const e of entries) {
			if (e.isDirectory() && !NOT_SOURCE_DIRS.has(e.name)) {
				walk(path.join(dir, e.name));
			}
		}
	};
	const root = cppRootDir(project);
	if (fs.existsSync(root)) {
		walk(root);
	}
	return result;
}

/**
 * 雛形の中身(DESIGN.md 3.2 章)。保存時の整形(.clang-format)で変わらない形にしてある。
 * 改行は LF(プロジェクトの files.eol)。BOM は書くときに付ける。
 */
export function cppFileContents(kind: CppFileKind, name: string, folder: string): { file: string; text: string }[] {
	const header = { file: path.join(folder, `${name}.h`), text: '#pragma once\n' };
	const includeOwnHeader = kind === 'class' || fs.existsSync(path.join(folder, `${name}.h`));
	const source = { file: path.join(folder, `${name}.cpp`), text: '#include "DxLib.h"\n' + (includeOwnHeader ? `#include "${name}.h"\n` : '') };
	if (kind === 'cpp') {
		return [source];
	}
	if (kind === 'h') {
		return [header];
	}
	header.text = ['#pragma once', '', `class ${name}`, '{', 'public:', `\t${name}();`, `\t~${name}();`, '};', ''].join('\n');
	source.text += ['', `${name}::${name}()`, '{', '}', '', `${name}::~${name}()`, '{', '}', ''].join('\n');
	return [header, source];
}

/** C++ のファイルを作る(ダイアログは出さない)。 */
export function createCppFiles(project: vscode.WorkspaceFolder, kind: CppFileKind, args: NewCppArgs): NewFilesResult {
	const name = args.name.trim();
	if (!NAME_RE.test(name)) {
		return { ok: false, error: NAME_ERROR };
	}
	const folder = args.folder;
	const vsp = vsProjectOf(project);
	if (!isInsideOrSame(folder, cppRootDir(project))) {
		return { ok: false, error: vsp ? 'C++ のファイルはプロジェクトのフォルダの中に作ってください。' : 'C++ のファイルは src フォルダの中に作ってください(src の外の .cpp はビルドされません)。' };
	}
	const contents = cppFileContents(kind, name, folder);
	for (const c of contents) {
		if (fs.existsSync(c.file)) {
			return { ok: false, error: `同じ名前のファイルが既にあります: ${path.basename(c.file)}` };
		}
	}
	fs.mkdirSync(folder, { recursive: true });
	for (const c of contents) {
		fs.writeFileSync(c.file, '﻿' + c.text, 'utf8'); // BOM 付き UTF-8
	}
	// Visual Studio のプロジェクトは .vcxproj にファイルの一覧を持つので、足したファイルを書き足す(DESIGN.md 6.1 章)
	if (vsp) {
		addToVsProject(vsp.vcxproj, contents.map((c) => c.file));
	}
	return { ok: true, files: contents.map((c) => c.file) };
}

/**
 * 作る場所。右クリックならそのフォルダ。見出しのボタン(引数なし)なら、
 * 今エディタで開いているファイルのフォルダ(src の下のとき)、無ければ src。
 * 拡張からはエクスプローラーで選んでいるフォルダを読めないため。
 */
function targetFolder(project: vscode.WorkspaceFolder, arg: unknown): string {
	if (arg instanceof vscode.Uri) {
		return arg.fsPath;
	}
	const editor = vscode.window.activeTextEditor;
	if (editor && editor.document.uri.scheme === 'file') {
		const dir = path.dirname(editor.document.uri.fsPath);
		if (isInsideOrSame(dir, cppRootDir(project))) {
			return dir;
		}
	}
	return cppRootDir(project);
}

/** 欄のフォームを開く処理(projectView)。作る場所を渡す。 */
export type OpenCppForm = (kind: CppFileKind, folder: string) => Promise<void>;

async function openFiles(files: string[]): Promise<void> {
	// クラスは .h を開く(先頭が .h になっている)
	await vscode.window.showTextDocument(vscode.Uri.file(files[0]));
}

export function registerNewFileCommands(context: vscode.ExtensionContext, openCppForm: OpenCppForm, openShaderForm: () => Promise<void>): void {
	const cppCommand = (kind: CppFileKind) => async (arg?: unknown): Promise<void> => {
		const project = currentFolder();
		if (!project) {
			void vscode.window.showWarningMessage('プロジェクトのフォルダが開かれていません。');
			return;
		}
		// 欄のフォームからの送信: 名前と場所が来たら作る
		if (arg && !(arg instanceof vscode.Uri)) {
			const result = createCppFiles(project, kind, arg as NewCppArgs);
			if (!result.ok) {
				void vscode.window.showErrorMessage(result.error ?? '不明なエラーです。');
				return;
			}
			await openFiles(result.files as string[]);
			return;
		}
		// 欄のボタン・右クリック(フォルダの Uri)・見出しのボタン(引数なし): 欄のフォームを開いて名前を聞く
		await openCppForm(kind, targetFolder(project, arg));
	};
	context.subscriptions.push(
		vscode.commands.registerCommand('dxlib.newCppSource', cppCommand('cpp')),
		vscode.commands.registerCommand('dxlib.newHeader', cppCommand('h')),
		vscode.commands.registerCommand('dxlib.newClass', cppCommand('class')),
		// シェーダー: 欄のシェーダーフォーム(種類と名前)を開く。作るのは dxlib.newShaderFile(フォームの送信先)
		vscode.commands.registerCommand('dxlib.addShader', async () => {
			if (!currentFolder()) {
				void vscode.window.showWarningMessage('プロジェクトのフォルダが開かれていません。');
				return;
			}
			await openShaderForm();
		}),
	);
}
