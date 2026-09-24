import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { CLANG_FORMAT, CLANG_FORMAT_V1 } from './createProject';

/**
 * 以前のこの拡張機能で作ったプロジェクトの設定を、今の形に直す(DESIGN.md 6・10 章)。
 * 直したものの説明を返す(何も直さなければ空)。
 *
 * - tasks.json の dxlib タスクの problemMatcher: 当初は ["$msCompile"] を書いていた。
 *   残っていると VSCode 側の消えない赤線がビルドのたびに付くので [] にする。
 * - .clang-format: 当初は UseTab: Always で、揃えにタブと空白が混ざった。
 *   当初の内容のまま(手で直していない)なら今の内容にする。
 */
export function migrateProject(folder: vscode.WorkspaceFolder): string[] {
	const changes: string[] = [];
	migrateTasks(folder.uri.fsPath, changes);
	migrateClangFormat(folder.uri.fsPath, changes);
	return changes;
}

function migrateTasks(projectDir: string, changes: string[]): void {
	const tasksFile = path.join(projectDir, '.vscode', 'tasks.json');
	let json: { tasks?: Array<{ type?: string; problemMatcher?: unknown }> };
	try {
		// 生成した tasks.json はコメントの無い JSON。手で書き換えられて読めないときは触らない。
		json = JSON.parse(fs.readFileSync(tasksFile, 'utf8').replace(/^﻿/, ''));
	} catch {
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
	if (text.replace(/\r\n/g, '\n') === CLANG_FORMAT_V1) {
		fs.writeFileSync(file, CLANG_FORMAT, 'utf8');
		changes.push('.clang-format を更新しました(保存時の整形で、行末コメントなどの揃えにタブと空白が混ざらなくなります)');
	}
}
