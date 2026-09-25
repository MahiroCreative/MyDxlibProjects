import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';

/**
 * ビルドエラーの赤線(DESIGN.md 6 章)。
 * タスクの問題マッチャー($msCompile)で付けた赤線は VSCode が持っていて拡張から消せず、
 * エラーを直しても次のビルドまで残る。そこで、ビルド用 bat が残した MSBuild(cl)の出力のログを
 * ビルド後に読んで自前で赤線を付け、そのファイルを書き換え始めたら消す。
 */

/** MSVC の `ファイル(行[,列]): [fatal ]error|warning Cxxxx: 内容`。$msCompile と同じ形。 */
const MSVC_LINE = /^(?:\s*\d+>)?\s*(\S.*?)\((\d+)(?:,(\d+))?\)\s*:\s+(?:fatal\s+)?(error|warning|info)\s+(\w{1,2}\d+)\s*:\s*(.*)$/;

export interface ParsedDiagnostic {
	file: string;
	line: number;
	column: number;
	severity: 'error' | 'warning' | 'info';
	code: string;
	message: string;
}

/** cl の出力を解析する。相対パスは baseDir から解決する。リンカーエラー(ファイルと行が無い)は含めない。 */
export function parseMsvcOutput(text: string, baseDir: string): ParsedDiagnostic[] {
	const result: ParsedDiagnostic[] = [];
	const seen = new Set<string>();
	for (const raw of text.split(/\r?\n/)) {
		const m = MSVC_LINE.exec(raw);
		if (!m) {
			continue;
		}
		const file = path.isAbsolute(m[1]) ? m[1] : path.resolve(baseDir, m[1]);
		// MSBuild は行末に「 [<名前>.vcxproj]」を付ける(DESIGN.md 6 章)
		const message = m[6].replace(/\s+\[[^\]]*\.vcxproj\]\s*$/i, '');
		const d: ParsedDiagnostic = {
			file,
			line: Math.max(1, Number(m[2])),
			column: m[3] ? Math.max(1, Number(m[3])) : 1,
			severity: m[4] as ParsedDiagnostic['severity'],
			code: m[5],
			message,
		};
		// MSBuild は同じエラーを最後にもう一度まとめて出すことがあるので、重ねない
		const key = `${d.file.toLowerCase()}|${d.line}|${d.column}|${d.code}|${d.message}`;
		if (!seen.has(key)) {
			seen.add(key);
			result.push(d);
		}
	}
	return result;
}

export class BuildDiagnostics implements vscode.Disposable {
	private readonly collection = vscode.languages.createDiagnosticCollection('dxlib-build');
	private readonly subscriptions: vscode.Disposable[] = [];

	/** @param logPathOf タスクからログファイルのパスを求める(ビルド用 bat と同じ場所) */
	constructor(private readonly logPathOf: (task: vscode.Task) => string | undefined) {
		this.subscriptions.push(
			this.collection,
			vscode.tasks.onDidStartTaskProcess((e) => {
				if (e.execution.task.definition.type === 'dxlib') {
					this.collection.clear();
				}
			}),
			vscode.tasks.onDidEndTaskProcess((e) => {
				if (e.execution.task.definition.type === 'dxlib') {
					this.loadFrom(e.execution.task);
				}
			}),
			// 直し始めたファイルのビルドエラーは消す(入力中の赤線は C/C++ 拡張が出す)
			vscode.workspace.onDidChangeTextDocument((e) => {
				if (e.contentChanges.length > 0 && this.collection.has(e.document.uri)) {
					this.collection.delete(e.document.uri);
				}
			}),
		);
	}

	private loadFrom(task: vscode.Task): void {
		this.collection.clear();
		const log = this.logPathOf(task);
		if (!log || !fs.existsSync(log)) {
			return;
		}
		const folder = task.scope && typeof task.scope === 'object' ? (task.scope as vscode.WorkspaceFolder).uri.fsPath : process.cwd();
		// 同じファイルでも大文字小文字が違うことがある(Windows)ので、小文字でまとめる
		const byFile = new Map<string, { uri: vscode.Uri; diags: vscode.Diagnostic[] }>();
		for (const d of parseMsvcOutput(fs.readFileSync(log, 'utf8'), folder)) {
			const range = new vscode.Range(d.line - 1, d.column - 1, d.line - 1, Number.MAX_SAFE_INTEGER);
			const severity = d.severity === 'error' ? vscode.DiagnosticSeverity.Error : d.severity === 'warning' ? vscode.DiagnosticSeverity.Warning : vscode.DiagnosticSeverity.Information;
			const diag = new vscode.Diagnostic(range, d.message, severity);
			diag.source = 'cl';
			diag.code = d.code;
			const key = d.file.toLowerCase();
			const entry = byFile.get(key) ?? { uri: vscode.Uri.file(d.file), diags: [] };
			entry.diags.push(diag);
			byFile.set(key, entry);
		}
		for (const { uri, diags } of byFile.values()) {
			this.collection.set(uri, diags);
		}
	}

	dispose(): void {
		for (const s of this.subscriptions) {
			s.dispose();
		}
	}
}
