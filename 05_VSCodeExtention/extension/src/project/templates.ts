import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';

export interface TemplateInfo {
	/** builtin:<folder> または ext:<folder> */
	id: string;
	name: string;
	description: string;
	dir: string;
	builtin: boolean;
}

interface TemplateJson {
	name?: string;
	description?: string;
}

function readTemplateJson(dir: string): TemplateJson | undefined {
	const file = path.join(dir, 'template.json');
	if (!fs.existsSync(file)) {
		return undefined;
	}
	try {
		return JSON.parse(fs.readFileSync(file, 'utf8').replace(/^\uFEFF/, '')) as TemplateJson;
	} catch {
		return undefined;
	}
}

function scan(root: string, builtin: boolean): TemplateInfo[] {
	if (!root || !fs.existsSync(root)) {
		return [];
	}
	const result: TemplateInfo[] = [];
	const names = fs
		.readdirSync(root, { withFileTypes: true })
		.filter((e) => e.isDirectory())
		.map((e) => e.name)
		.sort((a, b) => a.localeCompare(b, 'ja'));
	for (const name of names) {
		const dir = path.join(root, name);
		const json = readTemplateJson(dir);
		if (!json) {
			continue; // template.json が無いフォルダはテンプレートとみなさない
		}
		result.push({
			id: `${builtin ? 'builtin' : 'ext'}:${name}`,
			name: json.name?.trim() || name,
			description: json.description?.trim() ?? '',
			dir,
			builtin,
		});
	}
	return result;
}

/** 同梱テンプレート(先頭固定)+ 外部テンプレート(フォルダ名順)。 */
export function listTemplates(context: vscode.ExtensionContext, templatesPath: string): TemplateInfo[] {
	return [...scan(path.join(context.extensionPath, 'templates'), true), ...scan(templatesPath, false)];
}

export function findTemplate(context: vscode.ExtensionContext, templatesPath: string, id: string): TemplateInfo | undefined {
	return listTemplates(context, templatesPath).find((t) => t.id === id);
}
