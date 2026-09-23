import * as fs from 'fs';
import * as path from 'path';
import { currentFolder, getConfig, projectExeName } from '../env/environment';
import { copyProjectTree, isEmptyDir, writeText } from '../util/fsx';
import { PLACEHOLDER } from './createProject';

export interface SaveTemplateArgs {
	name: string;
	description: string;
	/** プロジェクト名を __PROJECT_NAME__ に戻すか。 */
	substitute: boolean;
}

export interface SaveTemplateResult {
	ok: boolean;
	error?: string;
	dest?: string;
	count?: number;
}

/**
 * 今開いているプロジェクトを外部テンプレートフォルダにコピーして template.json を作る
 * (パネル内フォームの送信先。ダイアログは出さない)。
 * 「プロジェクトのフォルダが開いているか」「テンプレートフォルダが設定済みか」は
 * コマンドの入口(extension.ts)側で確認済みの前提。
 */
export async function saveAsTemplateWork(args: SaveTemplateArgs): Promise<SaveTemplateResult> {
	const folder = currentFolder();
	if (!folder) {
		return { ok: false, error: 'プロジェクトのフォルダが開かれていません。' };
	}
	const templatesPath = getConfig<string>('templatesPath', '');
	if (!templatesPath || !fs.existsSync(templatesPath)) {
		return { ok: false, error: 'テンプレートフォルダが設定されていません。先に指定してください。' };
	}
	const name = args.name.trim();
	if (!name) {
		return { ok: false, error: '名前を入力してください。' };
	}

	const projectName = projectExeName(folder);
	const folderName = name.replace(/[\\/:*?"<>|]/g, '_');
	const dest = path.join(templatesPath, folderName);
	if (!isEmptyDir(dest)) {
		return { ok: false, error: `同じ名前のテンプレートが既にあります: ${dest}` };
	}

	const reverse = (s: string): string => (args.substitute ? s.split(projectName).join(PLACEHOLDER) : s);
	const count = copyProjectTree(folder.uri.fsPath, dest, {
		renameEntry: reverse,
		transformText: (text) => reverse(text),
	});
	writeText(path.join(dest, 'template.json'), JSON.stringify({ name, description: args.description.trim() }, null, '\t') + '\n');
	return { ok: true, dest, count };
}
