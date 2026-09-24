import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';

/**
 * 以前のこの拡張機能で作ったプロジェクトの設定を、今の形に直す(DESIGN.md 6 章)。
 * 直したものの説明を返す(何も直さなければ空)。
 *
 * - tasks.json の dxlib タスクの problemMatcher: 当初は ["$msCompile"] を書いていた。
 *   残っていると VSCode 側の消えない赤線がビルドのたびに付くので [] にする。
 */
export function migrateProject(folder: vscode.WorkspaceFolder): string[] {
	const changes: string[] = [];
	const tasksFile = path.join(folder.uri.fsPath, '.vscode', 'tasks.json');
	let json: { tasks?: Array<{ type?: string; problemMatcher?: unknown }> };
	try {
		// 生成した tasks.json はコメントの無い JSON。手で書き換えられて読めないときは触らない。
		json = JSON.parse(fs.readFileSync(tasksFile, 'utf8').replace(/^﻿/, ''));
	} catch {
		return changes;
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
	return changes;
}
