import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { getConfig } from '../env/environment';
import { inspectSdk, readSdkHeader } from '../env/sdk';

interface Declaration {
	/** 宣言の 1 行(余分な空白を詰めたもの)。 */
	signature: string;
	/** 行末の作者コメント。 */
	comment: string;
}

interface DxLibIndex {
	version: string;
	functions: Map<string, Declaration[]>;
	/** 関数名 → help 内の URL(file:///...html#RnNm)。 */
	reference: Map<string, vscode.Uri>;
}

const DOC_SCHEME = 'dxlib-doc';
const MAX_INLINE = 3;

let index: DxLibIndex | undefined;

/** DxLib.h の `extern 型 名前( 引数 ) ; // コメント` を関数名で索引にする。 */
function buildIndex(sdkPath: string, version: string, helpDir: string | undefined): DxLibIndex {
	const functions = new Map<string, Declaration[]>();
	const header = readSdkHeader(sdkPath, 'DxLib.h') ?? '';
	const declRe = /^\s*extern\s+(.+?)\s*;\s*(?:\/\/\s*(.*))?$/;
	const nameRe = /\b([A-Za-z_][A-Za-z0-9_]*)\s*\(/;
	for (const raw of header.split(/\r?\n/)) {
		const m = declRe.exec(raw);
		if (!m) {
			continue;
		}
		const body = m[1].replace(/\s+/g, ' ').trim();
		const n = nameRe.exec(body);
		if (!n) {
			continue;
		}
		const name = n[1];
		const list = functions.get(name) ?? [];
		list.push({ signature: body + ' ;', comment: (m[2] ?? '').trim() });
		functions.set(name, list);
	}

	const reference = new Map<string, vscode.Uri>();
	if (helpDir) {
		const indexFile = path.join(helpDir, 'dxfunc.html');
		if (fs.existsSync(indexFile)) {
			const html = fs.readFileSync(indexFile, 'utf8');
			const linkRe = /href="([^"]*#R\d+N\d+)"[^>]*>\s*([A-Za-z_][A-Za-z0-9_]*)\s*</g;
			let lm: RegExpExecArray | null;
			while ((lm = linkRe.exec(html)) !== null) {
				const [, href, name] = lm;
				const [file, fragment] = href.split('#');
				const target = file ? path.join(helpDir, file) : indexFile;
				if (!reference.has(name)) {
					reference.set(name, vscode.Uri.file(target).with({ fragment }));
				}
			}
		}
	}
	return { version, functions, reference };
}

function getIndex(): DxLibIndex | undefined {
	const sdk = inspectSdk(getConfig<string>('sdkPath', '') || undefined);
	if (!sdk?.ok || !sdk.version) {
		return undefined;
	}
	if (!index || index.version !== sdk.version) {
		index = buildIndex(sdk.path, sdk.version, sdk.helpDir);
	}
	return index;
}

export function invalidateDxLibIndex(): void {
	index = undefined;
}

function wordAt(document: vscode.TextDocument, position: vscode.Position): string | undefined {
	const range = document.getWordRangeAtPosition(position, /[A-Za-z_][A-Za-z0-9_]*/);
	return range ? document.getText(range) : undefined;
}

class DxLibHoverProvider implements vscode.HoverProvider {
	provideHover(document: vscode.TextDocument, position: vscode.Position): vscode.Hover | undefined {
		const idx = getIndex();
		const name = wordAt(document, position);
		if (!idx || !name) {
			return undefined;
		}
		const decls = idx.functions.get(name);
		if (!decls || decls.length === 0) {
			return undefined;
		}
		const md = new vscode.MarkdownString(undefined, true);
		md.isTrusted = true;
		const comment = decls.find((d) => d.comment)?.comment;
		md.appendMarkdown(`**DxLib** \`${name}\`` + (comment ? `  \n${comment}` : '') + '\n\n');
		for (const d of decls.slice(0, MAX_INLINE)) {
			md.appendCodeblock(d.signature, 'cpp');
		}
		const links: string[] = [];
		if (decls.length > MAX_INLINE) {
			const arg = encodeURIComponent(JSON.stringify([name]));
			links.push(`[残り ${decls.length - MAX_INLINE} 件を表示](command:dxlib.showAllDeclarations?${arg})`);
		}
		if (idx.reference.has(name)) {
			const arg = encodeURIComponent(JSON.stringify([name]));
			links.push(`[リファレンスを開く](command:dxlib.openReference?${arg})`);
		}
		if (links.length > 0) {
			md.appendMarkdown(links.join('  |  '));
		}
		return new vscode.Hover(md);
	}
}

class DxLibDocProvider implements vscode.TextDocumentContentProvider {
	provideTextDocumentContent(uri: vscode.Uri): string {
		const name = uri.path.replace(/^\//, '').replace(/\.cpp$/, '');
		const idx = getIndex();
		const decls = idx?.functions.get(name) ?? [];
		const lines = [`// DxLib ${idx?.version ?? ''}  ${name} の宣言 (${decls.length} 件)`, ''];
		for (const d of decls) {
			if (d.comment) {
				lines.push(`// ${d.comment}`);
			}
			lines.push(d.signature, '');
		}
		return lines.join('\n');
	}
}

export function registerDxLibHover(context: vscode.ExtensionContext): void {
	context.subscriptions.push(
		vscode.languages.registerHoverProvider([{ language: 'cpp' }, { language: 'c' }], new DxLibHoverProvider()),
		vscode.workspace.registerTextDocumentContentProvider(DOC_SCHEME, new DxLibDocProvider()),
		vscode.commands.registerCommand('dxlib.showAllDeclarations', async (name?: string) => {
			const target = name ?? (vscode.window.activeTextEditor ? wordAt(vscode.window.activeTextEditor.document, vscode.window.activeTextEditor.selection.active) : undefined);
			if (!target) {
				return;
			}
			const uri = vscode.Uri.parse(`${DOC_SCHEME}:/${target}.cpp`);
			const doc = await vscode.workspace.openTextDocument(uri);
			await vscode.window.showTextDocument(doc, { preview: true, viewColumn: vscode.ViewColumn.Beside });
		}),
		vscode.commands.registerCommand('dxlib.openReference', async (name?: string) => {
			const editor = vscode.window.activeTextEditor;
			const target = name ?? (editor ? wordAt(editor.document, editor.selection.active) : undefined);
			const idx = getIndex();
			if (!idx) {
				void vscode.window.showWarningMessage('DxLib SDK が設定されていないため、リファレンスを開けません。');
				return;
			}
			if (!target || !idx.reference.has(target)) {
				void vscode.window.showInformationMessage(target ? `「${target}」はリファレンスの目次にありません。` : 'カーソルを DxLib の関数名に置いてください。');
				return;
			}
			await vscode.env.openExternal(idx.reference.get(target) as vscode.Uri);
		}),
	);
}
