import { execFile } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';

/**
 * HLSL(.hlsl/.fx)の整形。C/C++ 拡張が同梱している clang-format.exe を借用する。
 * 新しいバイナリは配らない。プロジェクト直下の .clang-format(C++ と共通)をそのまま使う。
 * HLSL 独自の構文(cbuffer、register(t0)、SV_POSITION などのセマンティクス)は
 * clang-format には C++ の延長として認識され、実機で意図どおり整形されることを確認済み。
 */

let cachedClangFormatPath: string | null | undefined; // undefined = 未探索, null = 見つからなかった
let warnedMissing = false;

function findClangFormat(): string | undefined {
	if (cachedClangFormatPath !== undefined) {
		return cachedClangFormatPath ?? undefined;
	}
	const cpptools = vscode.extensions.getExtension('ms-vscode.cpptools');
	const candidate = cpptools ? path.join(cpptools.extensionPath, 'LLVM', 'bin', 'clang-format.exe') : undefined;
	cachedClangFormatPath = candidate && fs.existsSync(candidate) ? candidate : null;
	return cachedClangFormatPath ?? undefined;
}

function runClangFormat(clangFormatPath: string, text: string, cwd: string, filename: string): Promise<string> {
	return new Promise((resolve, reject) => {
		const child = execFile(clangFormatPath, ['-style=file', `-assume-filename=${filename}`], { cwd, maxBuffer: 16 * 1024 * 1024 }, (err, stdout, stderr) => {
			if (err) {
				reject(new Error(stderr || err.message));
			} else {
				resolve(stdout);
			}
		});
		child.stdin?.end(text, 'utf8');
	});
}

class HlslFormattingProvider implements vscode.DocumentFormattingEditProvider {
	async provideDocumentFormattingEdits(document: vscode.TextDocument): Promise<vscode.TextEdit[] | undefined> {
		const clangFormatPath = findClangFormat();
		if (!clangFormatPath) {
			if (!warnedMissing) {
				warnedMissing = true;
				void vscode.window.showWarningMessage('HLSL の自動整形には C/C++ 拡張(ms-vscode.cpptools)が必要です。整形をスキップしました。');
			}
			return undefined;
		}
		const original = document.getText();
		let formatted: string;
		try {
			formatted = await runClangFormat(clangFormatPath, original, path.dirname(document.uri.fsPath), path.basename(document.uri.fsPath));
		} catch {
			// 構文が壊れている編集中などで失敗することがある。毎回警告は出さず、そのままにする。
			return undefined;
		}
		if (formatted === original) {
			return [];
		}
		const fullRange = new vscode.Range(document.positionAt(0), document.positionAt(original.length));
		return [vscode.TextEdit.replace(fullRange, formatted)];
	}
}

export function registerHlslFormatter(context: vscode.ExtensionContext): void {
	context.subscriptions.push(vscode.languages.registerDocumentFormattingEditProvider({ language: 'hlsl' }, new HlslFormattingProvider()));
}
