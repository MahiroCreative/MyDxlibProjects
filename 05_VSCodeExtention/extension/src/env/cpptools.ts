import * as vscode from 'vscode';

export const CPPTOOLS_ID = 'ms-vscode.cpptools';

/**
 * missing: 未導入または無効
 * preparing: C/C++ 拡張が起動中で、まだ設定を問い合わせてきていない
 * ready: 設定プロバイダーへ問い合わせが来た(補完が効く状態)
 * unresponsive: 一定時間待っても問い合わせが来ない
 */
export type CppToolsState = 'missing' | 'preparing' | 'ready' | 'unresponsive';

class IntelliSenseStateStore {
	private state: CppToolsState = 'missing';
	private readonly emitter = new vscode.EventEmitter<CppToolsState>();
	readonly onDidChange = this.emitter.event;

	get(): CppToolsState {
		return this.state;
	}

	set(next: CppToolsState): void {
		if (this.state !== next) {
			this.state = next;
			this.emitter.fire(next);
		}
	}
}

export const intelliSenseState = new IntelliSenseStateStore();

export function cpptoolsInstalled(): boolean {
	return vscode.extensions.getExtension(CPPTOOLS_ID) !== undefined;
}

/** 起動時に C/C++ 拡張の有無を確認し、無ければ警告を出す。 */
export async function warnIfCpptoolsMissing(): Promise<void> {
	if (cpptoolsInstalled()) {
		return;
	}
	intelliSenseState.set('missing');
	const open = '拡張機能を開く';
	const choice = await vscode.window.showWarningMessage(
		'C/C++ 拡張(ms-vscode.cpptools)が無効か未インストールです。コード補完とデバッグが使えません。',
		open,
	);
	if (choice === open) {
		await vscode.commands.executeCommand('workbench.extensions.search', `@id:${CPPTOOLS_ID}`);
	}
}
