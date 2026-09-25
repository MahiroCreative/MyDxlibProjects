import * as vscode from 'vscode';
import { currentFolder, projectExeName } from '../env/environment';
import { SaveTemplateArgs } from '../project/saveAsTemplate';
import { NewShaderArgs, SHADER_TEMPLATES } from '../shader/compileShaders';
import { BASE_CSS, makeNonce } from './webviewCommon';

/**
 * エクスプローラーの「DxLib」欄(DESIGN.md 3.1 章)。
 * プロジェクトの操作(ビルド・実行・デバッグ実行・シェーダー・テンプレート保存)のボタンと、
 * 「新しいシェーダー」「テンプレートとして保存」のフォームを置く。
 * DxLib プロジェクトを開いていて信頼されているときだけ表示される(package.json の when)。
 */
export class DxLibProjectViewProvider implements vscode.WebviewViewProvider {
	static readonly viewType = 'dxlib.projectView';
	private view: vscode.WebviewView | undefined;
	/** 欄がまだ作られていないうちに頼まれたフォーム。作られたら開く。 */
	private pendingForm: 'shader' | 'template' | undefined;

	constructor(private readonly context: vscode.ExtensionContext) {}

	resolveWebviewView(view: vscode.WebviewView): void {
		this.view = view;
		view.webview.options = { enableScripts: true, localResourceRoots: [this.context.extensionUri] };
		view.webview.html = this.html(view.webview);
		view.webview.onDidReceiveMessage((m: { command: string; args?: unknown }) => void this.onMessage(m));
	}

	private state(): { projectName: string; shaderKinds: { id: string; label: string; description: string; suffix: string }[] } {
		const folder = currentFolder();
		return {
			projectName: folder ? projectExeName(folder) : '',
			shaderKinds: SHADER_TEMPLATES.map((t) => ({ id: t.id, label: t.label, description: t.description, suffix: t.suffix })),
		};
	}

	private async openForm(form: 'shader' | 'template'): Promise<void> {
		// 欄を表示する(エクスプローラーを開き、欄を展開してフォーカス)。
		// 欄の中身は隠れている間に作り直されることがあり、そのとき送った指示は届かない。
		// 欄から「開いた」(formOpened)が返るまで持っておき、作り直されたら(ready)改めて送る。
		this.pendingForm = form;
		await vscode.commands.executeCommand(`${DxLibProjectViewProvider.viewType}.focus`);
		if (this.view?.visible) {
			void this.view.webview.postMessage({ type: 'openForm', form, state: this.state() });
		}
	}

	/** 「新しいシェーダー」フォームを開く。 */
	async showNewShaderForm(): Promise<void> {
		await this.openForm('shader');
	}

	/** 「テンプレートとして保存」フォームを開く。 */
	async showSaveTemplateForm(): Promise<void> {
		await this.openForm('template');
	}

	private async onMessage(m: { command: string; args?: unknown }): Promise<void> {
		switch (m.command) {
			case 'ready':
				void this.view?.webview.postMessage({ type: 'status', state: this.state() });
				if (this.pendingForm) {
					void this.view?.webview.postMessage({ type: 'openForm', form: this.pendingForm, state: this.state() });
				}
				return;
			case 'formOpened':
				this.pendingForm = undefined;
				return;
			case 'newShaderFile':
				await vscode.commands.executeCommand('dxlib.newShaderFile', m.args as NewShaderArgs);
				return;
			case 'saveAsTemplate':
				await vscode.commands.executeCommand('dxlib.saveAsTemplate', m.args as SaveTemplateArgs);
				return;
			case 'openShaderForm':
				await this.showNewShaderForm();
				return;
			case 'openTemplateForm':
				// テンプレートフォルダの確認などは、コマンドの入口(引数なし)で行う
				await vscode.commands.executeCommand('dxlib.saveAsTemplate');
				return;
			default:
				// build / run / debug / compileShaders
				await vscode.commands.executeCommand(`dxlib.${m.command}`);
		}
	}

	private html(webview: vscode.Webview): string {
		const nonce = makeNonce();
		return /* html */ `<!DOCTYPE html>
<html lang="ja">
<head>
<meta charset="UTF-8">
<meta http-equiv="Content-Security-Policy" content="default-src 'none'; style-src 'unsafe-inline' ${webview.cspSource}; script-src 'nonce-${nonce}';">
<style>
${BASE_CSS}
	body { padding-top: 4px; }
	h2:first-child { margin-top: 4px; }
	#shader-form, #template-form { display: none; }
</style>
</head>
<body>
	<div id="main">
		<h2>ビルド・実行</h2>
		<div class="actions">
			<button data-cmd="build">ビルド</button>
			<button data-cmd="run">実行</button>
			<button data-cmd="debug">デバッグ実行</button>
		</div>
		<h2>シェーダー</h2>
		<div class="actions">
			<button class="secondary" id="btn-open-shader-form">新しいシェーダー</button>
			<button class="secondary" data-cmd="compileShaders">すべてコンパイル</button>
		</div>
		<h2>テンプレート</h2>
		<div class="actions">
			<button class="secondary" id="btn-open-template-form">テンプレートとして保存</button>
		</div>
	</div>

	<div id="shader-form">
		<h2>新しいシェーダーを作成</h2>
		<label>種類</label>
		<div id="sf-kinds"></div>
		<label for="sf-name">名前(英数字と _)</label>
		<input type="text" id="sf-name" value="MyShader">
		<div class="hint" id="sf-filename">ファイル名: </div>
		<div class="error" id="sf-error"></div>
		<div class="actions"><button id="btn-shader-create">作成</button><button class="secondary" id="btn-shader-cancel">キャンセル</button></div>
	</div>

	<div id="template-form">
		<h2>テンプレートとして保存</h2>
		<label for="tf-name">表示名</label>
		<input type="text" id="tf-name">
		<label for="tf-desc">説明(省略可)</label>
		<input type="text" id="tf-desc">
		<div class="checkbox-row"><input type="checkbox" id="tf-substitute" checked><label for="tf-substitute" style="margin:0;">プロジェクト名を __PROJECT_NAME__ に戻す</label></div>
		<div class="hint">別名で新規プロジェクトを作ったとき、ウィンドウタイトルなどがその名前に追従します。</div>
		<div class="error" id="tf-error"></div>
		<div class="actions"><button id="btn-template-save">保存</button><button class="secondary" id="btn-template-cancel">キャンセル</button></div>
	</div>

<script nonce="${nonce}">
	const vscode = acquireVsCodeApi();
	const $ = (id) => document.getElementById(id);
	let state = { projectName: '', shaderKinds: [] };

	document.querySelectorAll('[data-cmd]').forEach((b) => b.addEventListener('click', () => vscode.postMessage({ command: b.dataset.cmd })));

	function showScreen(name) {
		for (const id of ['main', 'shader-form', 'template-form']) { $(id).style.display = (id === name) ? 'block' : 'none'; }
	}

	// --- 新しいシェーダー ------------------------------------------------------
	$('btn-open-shader-form').addEventListener('click', () => vscode.postMessage({ command: 'openShaderForm' }));
	$('btn-shader-cancel').addEventListener('click', () => showScreen('main'));
	$('btn-shader-create').addEventListener('click', () => {
		const name = $('sf-name').value.trim();
		const picked = document.querySelector('input[name=sfkind]:checked');
		if (!picked) { $('sf-error').textContent = '種類を選んでください。'; return; }
		if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(name)) { $('sf-error').textContent = '名前は英数字とアンダースコアだけで、先頭は英字か _ にしてください。'; return; }
		$('sf-error').textContent = '';
		showScreen('main');
		vscode.postMessage({ command: 'newShaderFile', args: { kindId: picked.value, name } });
	});

	function openShaderForm() {
		showScreen('shader-form');
		$('sf-error').textContent = '';
		renderShaderKinds();
		updateShaderFilenamePreview();
	}

	function renderShaderKinds() {
		const box = $('sf-kinds'); box.innerHTML = '';
		state.shaderKinds.forEach((k, i) => {
			const row = document.createElement('label'); row.className = 'tpl';
			const r = document.createElement('input'); r.type = 'radio'; r.name = 'sfkind'; r.value = k.id; if (i === 0) r.checked = true;
			r.addEventListener('change', updateShaderFilenamePreview);
			const txt = document.createElement('span'); txt.textContent = k.label;
			const small = document.createElement('small'); small.textContent = k.description;
			txt.appendChild(small);
			row.appendChild(r); row.appendChild(txt); box.appendChild(row);
		});
	}

	function updateShaderFilenamePreview() {
		const picked = document.querySelector('input[name=sfkind]:checked');
		const kind = state.shaderKinds.find((k) => picked && k.id === picked.value);
		const name = $('sf-name').value.trim() || '(名前)';
		$('sf-filename').textContent = 'ファイル名: ' + name + (kind ? kind.suffix : '') + '.hlsl';
	}
	$('sf-name').addEventListener('input', updateShaderFilenamePreview);

	// --- テンプレートとして保存 -------------------------------------------------
	$('btn-open-template-form').addEventListener('click', () => vscode.postMessage({ command: 'openTemplateForm' }));
	$('btn-template-cancel').addEventListener('click', () => showScreen('main'));
	$('btn-template-save').addEventListener('click', () => {
		const name = $('tf-name').value.trim();
		const description = $('tf-desc').value.trim();
		const substitute = $('tf-substitute').checked;
		if (!name) { $('tf-error').textContent = '名前を入力してください。'; return; }
		$('tf-error').textContent = '';
		showScreen('main');
		vscode.postMessage({ command: 'saveAsTemplate', args: { name, description, substitute } });
	});

	function openTemplateForm() {
		showScreen('template-form');
		$('tf-error').textContent = '';
		if (state.projectName && !$('tf-name').value) { $('tf-name').value = state.projectName; }
	}

	window.addEventListener('message', (e) => {
		const m = e.data;
		if (m.state) { state = m.state; }
		if (m.type === 'openForm') {
			if (m.form === 'shader') { openShaderForm(); } else { openTemplateForm(); }
			vscode.postMessage({ command: 'formOpened' });
		}
	});
	vscode.postMessage({ command: 'ready' });
</script>
</body>
</html>`;
	}
}
