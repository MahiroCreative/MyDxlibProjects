import { execFile } from 'child_process';

export interface ExecResult {
	code: number;
	stdout: string;
	stderr: string;
}

/** 外部プロセスを実行して終了コードと出力を返す。失敗しても例外は投げない。 */
export function run(file: string, args: string[], cwd?: string): Promise<ExecResult> {
	return new Promise((resolve) => {
		execFile(file, args, { cwd, windowsHide: true, maxBuffer: 16 * 1024 * 1024 }, (err, stdout, stderr) => {
			let code = 0;
			if (err) {
				const c = (err as NodeJS.ErrnoException & { code?: unknown }).code;
				code = typeof c === 'number' ? c : 1;
			}
			resolve({ code, stdout: String(stdout ?? ''), stderr: String(stderr ?? '') });
		});
	});
}

/**
 * Windows のコマンドライン規則(CommandLineToArgvW)どおりに 1 つの引数を引用する。
 * 空白・二重引用符・末尾のバックスラッシュを含んでも、受け取り側で元の文字列に戻る。
 */
export function quoteWindowsArg(a: string): string {
	if (a.length > 0 && !/[\s"]/.test(a)) {
		return a;
	}
	let out = '"';
	let backslashes = 0;
	for (const ch of a) {
		if (ch === '\\') {
			backslashes++;
		} else if (ch === '"') {
			out += '\\'.repeat(backslashes * 2 + 1) + '"';
			backslashes = 0;
		} else {
			out += '\\'.repeat(backslashes) + ch;
			backslashes = 0;
		}
	}
	return out + '\\'.repeat(backslashes * 2) + '"';
}

/** PowerShell の Start-Process に渡すスクリプトを組み立てる(検証用に分離)。 */
export function buildStartProcessScript(file: string, args: string[], extra: string): string {
	// Start-Process -ArgumentList に配列を渡すと、PowerShell 5.1 は空白を含む要素を引用せずに
	// 空白で連結する。"C:\Program Files\..." が 4 つの引数に割れるので、自前で 1 本の
	// コマンドライン文字列に組み立て、単一引用符の文字列として渡す(2026-09-23 実測で確認)。
	const line = args.map(quoteWindowsArg).join(' ');
	return `Start-Process -FilePath '${file.replace(/'/g, "''")}' -ArgumentList '${line.replace(/'/g, "''")}' ${extra}`.trim();
}

/** 管理者権限の確認(UAC)を出しながら実行ファイルを起動する。終了は待たない。 */
export function launchElevated(file: string, args: string[]): Promise<ExecResult> {
	return run('powershell.exe', ['-NoProfile', '-NonInteractive', '-Command', buildStartProcessScript(file, args, '-Verb RunAs')]);
}
