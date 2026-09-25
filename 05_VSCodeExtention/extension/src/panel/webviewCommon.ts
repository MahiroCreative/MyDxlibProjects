/** DxLib パネルとエクスプローラーの「DxLib」欄で共通の見た目。 */
export const BASE_CSS = `
	body { font-family: var(--vscode-font-family); font-size: var(--vscode-font-size); color: var(--vscode-foreground); padding: 0 8px 12px; }
	h2 { font-size: 12px; text-transform: none; opacity: 0.8; margin: 14px 0 6px; border-bottom: 1px solid var(--vscode-panel-border); padding-bottom: 3px; }
	.row { display: flex; align-items: flex-start; gap: 6px; margin: 6px 0; line-height: 1.4; }
	.row .text { flex: 1; min-width: 0; overflow-wrap: break-word; }
	.row .path { opacity: 0.7; font-size: 11px; display: block; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; cursor: default; }
	.ok { color: var(--vscode-charts-green, var(--vscode-testing-iconPassed)); }
	.ng { color: var(--vscode-errorForeground, var(--vscode-testing-iconFailed)); }
	.warn { color: var(--vscode-testing-iconQueued); }
	.wait { opacity: 0.7; }
	button { background: var(--vscode-button-background); color: var(--vscode-button-foreground); border: none; padding: 4px 10px; border-radius: 2px; cursor: pointer; font-size: 12px; white-space: nowrap; }
	button:hover { background: var(--vscode-button-hoverBackground); }
	/* 控えめなボタン(参照・キャンセルなど)も、文字だけに見えないよう枠を付ける(テーマによっては背景が無い) */
	button.secondary { background: var(--vscode-button-secondaryBackground); color: var(--vscode-button-secondaryForeground); border: 1px solid var(--vscode-button-border, var(--vscode-contrastBorder, rgba(128, 128, 128, 0.6))); }
	button.big { width: 100%; padding: 8px; font-size: 13px; margin: 6px 0; }
	button:disabled { opacity: 0.45; cursor: not-allowed; }
	button:disabled:hover { background: var(--vscode-button-background); }
	.actions { display: flex; flex-wrap: wrap; gap: 6px; margin-top: 6px; }
	input[type=text] { width: 100%; box-sizing: border-box; background: var(--vscode-input-background); color: var(--vscode-input-foreground); border: 1px solid var(--vscode-input-border, transparent); padding: 4px 6px; }
	label { display: block; margin: 8px 0 3px; }
	.checkbox-row { display: flex; align-items: center; gap: 6px; margin: 8px 0; }
	.checkbox-row input { margin: 0; }
	.tpl { display: flex; gap: 6px; align-items: flex-start; margin: 4px 0; }
	.tpl small { display: block; opacity: 0.7; }
	.hint { opacity: 0.7; font-size: 11px; margin-top: 3px; }
	.error { color: var(--vscode-errorForeground); margin-top: 6px; }
	.hidden { display: none !important; }
`;

/** Webview のスクリプトを許可するための nonce。 */
export function makeNonce(): string {
	return Array.from({ length: 16 }, () => Math.floor(Math.random() * 36).toString(36)).join('');
}
