import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import * as vscode from 'vscode';
import { readZip, ZipEntry } from '../util/zip';

/**
 * テンプレート(DESIGN.md 8 章)。
 * 同梱は拡張の中のフォルダ(templates/<名前>/)。授業用・自作はテンプレートファイル(.dxtemplate。中身は zip)1 つ = 1 テンプレート。
 * 手作りの .zip も同じ形なら使える。ファイルは作成フォームで選び、「最近使ったテンプレート」として覚える(拡張の保存フォルダのファイル。PC ごと)。
 */

export interface TemplateInfo {
	/** builtin:<フォルダ名> または file:<テンプレートファイルの絶対パス> */
	id: string;
	name: string;
	description: string;
	builtin: boolean;
	/** 同梱: フォルダ。file: テンプレートファイル。 */
	source: string;
}

interface TemplateJson {
	name?: string;
	description?: string;
}

const RECENT_MAX = 10;
/** テンプレートファイルの拡張子(保存するときに付ける。選ぶときは .zip も受け付ける)。 */
export const TEMPLATE_EXT = '.dxtemplate';
export const NOT_TEMPLATE_FILE = 'テンプレートのファイルではありません(中に template.json がありません)。';

function parseTemplateJson(text: string): TemplateJson | undefined {
	try {
		return JSON.parse(text.replace(/^﻿/, '')) as TemplateJson;
	} catch {
		return undefined;
	}
}

function scanBuiltin(root: string): TemplateInfo[] {
	if (!fs.existsSync(root)) {
		return [];
	}
	const result: TemplateInfo[] = [];
	for (const e of fs.readdirSync(root, { withFileTypes: true }).filter((d) => d.isDirectory())) {
		const dir = path.join(root, e.name);
		const file = path.join(dir, 'template.json');
		const json = fs.existsSync(file) ? parseTemplateJson(fs.readFileSync(file, 'utf8')) : undefined;
		if (!json) {
			continue;
		}
		result.push({ id: `builtin:${e.name}`, name: json.name?.trim() || e.name, description: json.description?.trim() ?? '', builtin: true, source: dir });
	}
	return result;
}

/** テンプレートファイル(zip の形式)を読んで、テンプレートとしての表示名・説明と中身を返す。テンプレートでなければ例外(理由は日本語)。 */
export function readTemplateZip(file: string): { info: TemplateInfo; entries: ZipEntry[] } {
	let entries: ZipEntry[];
	try {
		entries = readZip(fs.readFileSync(file));
	} catch (e) {
		throw new Error(`テンプレートのファイルを読めません: ${e instanceof Error ? e.message : String(e)}`);
	}
	const jsonEntry = entries.find((e) => e.name === 'template.json');
	const json = jsonEntry ? parseTemplateJson(jsonEntry.data.toString('utf8')) : undefined;
	if (!json) {
		throw new Error(NOT_TEMPLATE_FILE);
	}
	const base = path.basename(file, path.extname(file));
	return {
		info: { id: `file:${file}`, name: json.name?.trim() || base, description: json.description?.trim() ?? '', builtin: false, source: file },
		entries,
	};
}

function recentFile(context: vscode.ExtensionContext): string {
	return path.join(context.globalStorageUri.fsPath, 'recent-templates.json');
}

function readRecent(context: vscode.ExtensionContext): string[] {
	try {
		const list = JSON.parse(fs.readFileSync(recentFile(context), 'utf8'));
		return Array.isArray(list) ? list.filter((x): x is string => typeof x === 'string') : [];
	} catch {
		return [];
	}
}

/**
 * 「最近使ったテンプレート」の先頭に入れる。globalState は後でまとめて書かれるので、
 * 作成先の初期値と同じく、その場でファイルに書く(DESIGN.md 8 章)。
 */
export function addRecentTemplate(context: vscode.ExtensionContext, file: string): void {
	const same = (a: string): boolean => path.resolve(a).toLowerCase() === path.resolve(file).toLowerCase();
	const list = [file, ...readRecent(context).filter((f) => !same(f))].slice(0, RECENT_MAX);
	try {
		fs.mkdirSync(context.globalStorageUri.fsPath, { recursive: true });
		fs.writeFileSync(recentFile(context), JSON.stringify(list, null, '\t'), 'utf8');
	} catch {
		// 記録できなくても、テンプレートの保存・作成自体は成功している
	}
}

/** 同梱(先頭)+ 最近使ったテンプレートファイル(新しい順。消えた・読めないものは外す)。 */
export function listTemplates(context: vscode.ExtensionContext): TemplateInfo[] {
	const result = scanBuiltin(path.join(context.extensionPath, 'templates'));
	for (const file of readRecent(context)) {
		try {
			result.push(readTemplateZip(file).info);
		} catch {
			// 消えた・壊れたファイルは一覧に出さない
		}
	}
	return result;
}

export function findTemplate(context: vscode.ExtensionContext, id: string): TemplateInfo | undefined {
	if (id.startsWith('file:')) {
		try {
			return readTemplateZip(id.slice(5)).info;
		} catch {
			return undefined;
		}
	}
	return listTemplates(context).find((t) => t.id === id);
}

/**
 * テンプレートの中身があるフォルダを返す。テンプレートファイルは一時フォルダに展開する(使い終わったら cleanup を呼ぶ)。
 * ファイルの中の「..」や絶対パスの名前は、展開先の外に書かないよう拒否する。
 */
export function templateSourceDir(template: TemplateInfo): { dir: string; cleanup: () => void } {
	if (template.builtin) {
		return { dir: template.source, cleanup: () => undefined };
	}
	const { entries } = readTemplateZip(template.source);
	const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'dxlib-template-'));
	const cleanup = (): void => fs.rmSync(dir, { recursive: true, force: true });
	try {
		for (const e of entries) {
			const out = path.resolve(dir, e.name);
			if (!out.toLowerCase().startsWith(path.resolve(dir).toLowerCase() + path.sep)) {
				throw new Error(`テンプレートのファイルに使えない名前があります: ${e.name}`);
			}
			fs.mkdirSync(path.dirname(out), { recursive: true });
			fs.writeFileSync(out, e.data);
		}
	} catch (e) {
		cleanup();
		throw e;
	}
	return { dir, cleanup };
}
