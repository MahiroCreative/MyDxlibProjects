import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { ensureProjectFiles, projectFilePaths } from '../build/vcxproj';
import { vsProjectOf } from '../build/vsProject';
import { getConfig, projectExeName } from '../env/environment';
import { CLANG_FORMAT, OLD_CLANG_FORMATS } from './createProject';

/**
 * 以前のこの拡張機能で作ったプロジェクトの設定を、今の形に直す(DESIGN.md 3.2・6・10 章)。
 * 直したものの説明を返す(何も直さなければ空)。
 *
 * - tasks.json の dxlib タスクの problemMatcher: 当初は ["$msCompile"] を書いていた。
 *   残っていると VSCode 側の消えない赤線がビルドのたびに付くので [] にする。
 * - .clang-format: 当初は UseTab: Always で、揃えにタブと空白が混ざった。2026-09-24 の版は public: が空白 2 個の字下げになった。
 *   以前の内容のまま(手で直していない)なら今の内容にする。
 * - settings.json: C/C++ 拡張がエディタ右上に出す ▶ を消す設定(C_Cpp.debugShortcut: false)が無ければ足す。
 * - MSBuild 用の .vcxproj・.sln・dxlib.props(2026-09-25 から。DESIGN.md 6 章)が無ければ作る。dxlib.props は SDK の場所に合わせる。
 */
export function migrateProject(folder: vscode.WorkspaceFolder): string[] {
	const changes: string[] = [];
	migrateTasks(folder.uri.fsPath, changes);
	migrateSettings(folder.uri.fsPath, changes);
	// Visual Studio で作ったプロジェクトは、.clang-format もこの拡張の .vcxproj も作らない(DESIGN.md 6.1 章)
	if (!vsProjectOf(folder)) {
		migrateClangFormat(folder.uri.fsPath, changes);
		migrateMsbuild(folder, changes);
	}
	return changes;
}

/** 生成した JSON(コメントなし)を読む。手で書き換えられて読めないときは undefined(触らない)。 */
function readJson<T>(file: string): T | undefined {
	try {
		return JSON.parse(fs.readFileSync(file, 'utf8').replace(/^﻿/, '')) as T;
	} catch {
		return undefined;
	}
}

function migrateTasks(projectDir: string, changes: string[]): void {
	const tasksFile = path.join(projectDir, '.vscode', 'tasks.json');
	const json = readJson<{ tasks?: Array<{ type?: string; problemMatcher?: unknown }> }>(tasksFile);
	if (!json) {
		return;
	}
	let changed = false;
	for (const t of json.tasks ?? []) {
		if (t.type === 'dxlib' && !(Array.isArray(t.problemMatcher) && t.problemMatcher.length === 0)) {
			t.problemMatcher = [];
			changed = true;
		}
	}
	if (changed) {
		fs.writeFileSync(tasksFile, JSON.stringify(json, null, '\t') + '\n', 'utf8');
		changes.push('tasks.json のビルドエラーの表示方法を更新しました(エラーを直すと赤線がすぐ消えるようになります)');
	}
}

function migrateClangFormat(projectDir: string, changes: string[]): void {
	const file = path.join(projectDir, '.clang-format');
	let text: string;
	try {
		text = fs.readFileSync(file, 'utf8');
	} catch {
		return;
	}
	if (OLD_CLANG_FORMATS.includes(text.replace(/\r\n/g, '\n'))) {
		fs.writeFileSync(file, CLANG_FORMAT, 'utf8');
		changes.push('.clang-format を更新しました(保存時の整形で、タブと空白が混ざらなくなります)');
	}
}

function migrateSettings(projectDir: string, changes: string[]): void {
	const file = path.join(projectDir, '.vscode', 'settings.json');
	const json = readJson<Record<string, unknown>>(file);
	if (!json || 'C_Cpp.debugShortcut' in json) {
		return;
	}
	json['C_Cpp.debugShortcut'] = false;
	fs.writeFileSync(file, JSON.stringify(json, null, '\t') + '\n', 'utf8');
	changes.push('settings.json を更新しました(エディタ右上のボタンを DxLib のビルド・実行・デバッグ実行だけにします)');
}

function migrateMsbuild(folder: vscode.WorkspaceFolder, changes: string[]): void {
	const name = projectExeName(folder);
	const vcxproj = projectFilePaths(folder.uri.fsPath, name).vcxproj;
	const existed = fs.existsSync(vcxproj);
	const r = ensureProjectFiles(folder.uri.fsPath, name, getConfig<string>('sdkPath', ''), getConfig<string>('build.cppStandard', 'c++20', folder));
	if (!existed && r.written.some((f) => f === vcxproj)) {
		changes.push(`ビルドの仕組みを MSBuild にしました(${path.basename(vcxproj)} と .sln を作りました。Visual Studio でも開けます)`);
	}
}
