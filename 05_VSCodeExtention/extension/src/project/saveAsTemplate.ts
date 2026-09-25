import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import * as vscode from 'vscode';
import { currentFolder, projectExeName } from '../env/environment';
import { copyProjectTree } from '../util/fsx';
import { writeZip, ZipEntry } from '../util/zip';
import { PLACEHOLDER } from './createProject';
import { addRecentTemplate, TEMPLATE_EXT } from './templates';

export interface SaveTemplateArgs {
	name: string;
	description: string;
	/** プロジェクト名を __PROJECT_NAME__ に戻すか。 */
	substitute: boolean;
	/** 保存するテンプレートファイル(.dxtemplate)。省略すると、コマンドの入口で保存ダイアログを出して決める(検証では渡す)。 */
	file?: string;
}

export interface SaveTemplateResult {
	ok: boolean;
	error?: string;
	file?: string;
	count?: number;
}

function lastDirFile(context: vscode.ExtensionContext): string {
	return path.join(context.globalStorageUri.fsPath, 'last-template-dir.txt');
}

/** 保存ダイアログの初期値: 前回の保存先のフォルダ(無ければドキュメント)の「<表示名>.dxtemplate」。 */
export function defaultTemplateFile(context: vscode.ExtensionContext, name: string): string {
	let dir: string | undefined;
	try {
		dir = fs.readFileSync(lastDirFile(context), 'utf8').trim();
	} catch {
		dir = undefined;
	}
	if (!dir || !fs.existsSync(dir)) {
		dir = path.join(os.homedir(), 'Documents');
	}
	const base = name.trim().replace(/[\\/:*?"<>|]/g, '_') || 'template';
	return path.join(dir, `${base}${TEMPLATE_EXT}`);
}

/** テンプレートファイルに入れるファイルを集める(禁則は copyProjectTree と同じ)。一時フォルダに写してから読む。 */
function collectEntries(projectDir: string, reverse: (s: string) => string): { entries: ZipEntry[]; count: number } {
	const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'dxlib-save-'));
	try {
		const count = copyProjectTree(projectDir, tmp, { renameEntry: reverse, transformText: (text) => reverse(text) });
		const entries: ZipEntry[] = [];
		const walk = (dir: string, rel: string): void => {
			for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
				const full = path.join(dir, e.name);
				const name = rel ? `${rel}/${e.name}` : e.name;
				if (e.isDirectory()) {
					walk(full, name);
				} else if (e.isFile()) {
					entries.push({ name, data: fs.readFileSync(full) });
				}
			}
		};
		walk(tmp, '');
		return { entries, count };
	} finally {
		fs.rmSync(tmp, { recursive: true, force: true });
	}
}

/**
 * 今開いているプロジェクトをテンプレートファイル(.dxtemplate。中身は zip)に保存する(欄のフォームの送信先。ダイアログは出さない)。
 * 保存先(args.file)はコマンドの入口で保存ダイアログから決めてある前提。上書きの確認もダイアログが済ませている。
 */
export async function saveAsTemplateWork(context: vscode.ExtensionContext, args: SaveTemplateArgs & { file: string }): Promise<SaveTemplateResult> {
	const folder = currentFolder();
	if (!folder) {
		return { ok: false, error: 'プロジェクトのフォルダが開かれていません。' };
	}
	const name = args.name.trim();
	if (!name) {
		return { ok: false, error: '名前を入力してください。' };
	}
	const projectName = projectExeName(folder);
	const reverse = (s: string): string => (args.substitute ? s.split(projectName).join(PLACEHOLDER) : s);
	const { entries, count } = collectEntries(folder.uri.fsPath, reverse);
	entries.unshift({ name: 'template.json', data: Buffer.from(JSON.stringify({ name, description: args.description.trim() }, null, '\t') + '\n', 'utf8') });
	try {
		fs.mkdirSync(path.dirname(args.file), { recursive: true });
		fs.writeFileSync(args.file, writeZip(entries));
	} catch (e) {
		return { ok: false, error: `保存できませんでした: ${e instanceof Error ? e.message : String(e)}` };
	}
	try {
		fs.mkdirSync(context.globalStorageUri.fsPath, { recursive: true });
		fs.writeFileSync(lastDirFile(context), path.dirname(args.file), 'utf8');
	} catch {
		// 次の保存ダイアログの初期値がドキュメントになるだけ
	}
	addRecentTemplate(context, args.file);
	return { ok: true, file: args.file, count };
}
