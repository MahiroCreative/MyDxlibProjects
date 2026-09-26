import * as path from 'path';
import * as vscode from 'vscode';
import { currentFolder, projectExeName } from '../env/environment';
import { CppFileKind, KIND_LABEL } from '../project/newFiles';
import { SaveTemplateArgs } from '../project/saveAsTemplate';
import { NewShaderArgs, SHADER_TEMPLATES } from '../shader/compileShaders';
import { BASE_CSS, makeNonce } from './webviewCommon';

/**
 * エクスプローラーの「DxLib」欄(DESIGN.md 3.1 章)。
 * ソースコード(.cpp・.h・クラスの作成)・シェーダー(作成・すべてコンパイル)・テンプレート保存のボタンと、
 * 「C++ のファイルを作成」「新しいシェーダー」「テンプレートとして保存」のフォームを置く。
 * ファイルの作成は、右クリック・見出しのボタンから呼んでも、この欄のフォームで名前を聞く(DESIGN.md 3.2 章。
 * 画面上部の入力欄は生徒が UI と認識しにくいので使わない)。ビルド・実行・デバッグ実行はエディタ右上。
 * DxLib プロジェクトを開いていて信頼されているときだけ表示される(package.json の when)。
 */
export class DxLibProjectViewProvider implements vscode.WebviewViewProvider {
	static readonly viewType = 'dxlib.projectView';
	private view: vscode.WebviewView | undefined;
	/** 欄がまだ作られていないうちに頼まれたフォーム。作られたら開く。 */
	private pendingForm: FormRequest | undefined;
	/** 最後に頼まれたフォーム(検証用)。 */
	lastFormRequest: FormRequest | undefined;

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

	private async openForm(request: FormRequest): Promise<void> {
		// 欄を表示する(エクスプローラーを開き、欄を展開してフォーカス)。
		// 欄の中身は隠れている間に作り直されることがあり、そのとき送った指示は届かない。
		// 欄から「開いた」(formOpened)が返るまで持っておき、作り直されたら(ready)改めて送る。
		this.pendingForm = request;
		this.lastFormRequest = request;
		await vscode.commands.executeCommand(`${DxLibProjectViewProvider.viewType}.focus`);
		if (this.view?.visible) {
			void this.view.webview.postMessage({ type: 'openForm', ...request, state: this.state() });
		}
	}

	/** 「新しいシェーダー」フォームを開く。 */
	async showNewShaderForm(): Promise<void> {
		await this.openForm({ form: 'shader' });
	}

	/** 「テンプレートとして保存」フォームを開く。 */
	async showSaveTemplateForm(): Promise<void> {
		await this.openForm({ form: 'template' });
	}

	/** 「C++ のファイルを作成」フォームを開く(.cpp・.h・クラス)。folder は作る場所。 */
	async showCppForm(kind: CppFileKind, folder: string): Promise<void> {
		const project = currentFolder();
		const place = project ? path.relative(project.uri.fsPath, folder) || '.' : folder;
		await this.openForm({ form: 'cpp', kind, title: `${KIND_LABEL[kind]}を作成`, folder, place });
	}

	private async onMessage(m: { command: string; args?: unknown }): Promise<void> {
		switch (m.command) {
			case 'ready':
				void this.view?.webview.postMessage({ type: 'status', state: this.state() });
				if (this.pendingForm) {
					void this.view?.webview.postMessage({ type: 'openForm', ...this.pendingForm, state: this.state() });
				}
				return;
			case 'formOpened':
				this.pendingForm = undefined;
				return;
			case 'newCppFiles': {
				// フォームの送信。コマンドに名前と場所を渡すと作る(newFiles.ts)
				const a = m.args as { kind: CppFileKind; name: string; folder: string };
				const id = a.kind === 'cpp' ? 'dxlib.newCppSource' : a.kind === 'h' ? 'dxlib.newHeader' : 'dxlib.newClass';
				await vscode.commands.executeCommand(id, { name: a.name, folder: a.folder });
				return;
			}
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
				// プロジェクトが開いているかの確認は、コマンドの入口(引数なし)で行う
				await vscode.commands.executeCommand('dxlib.saveAsTemplate');
				return;
			default:
				// newCppSource / newHeader / newClass / addShader(引数なし = フォームを開く)/ compileShaders
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
	body { padding-top: 2px; }
	/* 見出しを左、ボタンを右に並べる(3 行に収める) */
	.rows { display: grid; grid-template-columns: max-content 1fr; column-gap: 8px; row-gap: 3px; align-items: center; margin-top: 2px; }
	.row-label { font-size: 11px; opacity: 0.8; white-space: nowrap; }
	.row-buttons { display: flex; flex-wrap: wrap; gap: 4px; }
	/* 欄の幅が狭くても 1 行に収まるよう、ボタンの左右の余白を少し詰める */
	.row-buttons button { padding: 3px 7px; }
	h2:first-child { margin-top: 4px; }
	#shader-form, #template-form, #cpp-form { display: none; }
	/* C++ のファイルの作成: 小さい欄に収まるよう、名前とボタンを 1 行に並べる */
	.form-title { font-size: 12px; font-weight: 600; margin: 6px 0 4px; }
	.inline-row { display: flex; gap: 4px; align-items: center; }
	.inline-row input { flex: 1; min-width: 60px; margin: 0; }
</style>
</head>
<body>
	<div id="main">
		<!-- ビルド・実行・デバッグ実行はエディタ右上、ファイルの追加は右クリックと見出しのボタンにもある(DESIGN.md 3.1・3.2 章)。
		     欄は小さく、拡張からは広げられないので、見出しを行の左に置いて 3 行に収める。 -->
		<div class="rows">
			<div class="row-label">ソースコード</div>
			<div class="row-buttons">
				<button data-cmd="newCppSource" title="C++ ソース (.cpp) を作成">.cpp</button>
				<button data-cmd="newHeader" title="ヘッダー (.h) を作成">.h</button>
				<button data-cmd="newClass" title="クラス (.h と .cpp の組) を作成">クラス</button>
			</div>
			<div class="row-label">シェーダー</div>
			<div class="row-buttons">
				<button data-cmd="addShader" title="シェーダーを作成(shaders に作る)">作成</button>
				<button data-cmd="compileShaders" title="shaders のシェーダーをすべてコンパイル">すべてコンパイル</button>
			</div>
			<div class="row-label">テンプレート</div>
			<div class="row-buttons">
				<button id="btn-open-template-form" title="今開いているプロジェクトをテンプレートとして保存">保存</button>
			</div>
			<div class="row-label">リリース</div>
			<div class="row-buttons">
				<button data-cmd="packageRelease" title="Release でビルドして、exe と素材を dist フォルダ(と zip)にまとめる">配布用にまとめる</button>
			</div>
		</div>
	</div>

	<div id="cpp-form">
		<div class="form-title" id="cf-title"></div>
		<div class="inline-row"><input type="text" id="cf-name" placeholder="名前(例: Player)"><button id="btn-cpp-create">作成</button><button class="secondary" id="btn-cpp-cancel">キャンセル</button></div>
		<div class="hint" id="cf-file"></div>
		<div class="error" id="cf-error"></div>
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
		for (const id of ['main', 'shader-form', 'template-form', 'cpp-form']) { $(id).style.display = (id === name) ? 'block' : 'none'; }
	}

	// --- C++ のファイルの作成(.cpp・.h・クラス) ----------------------------------
	let cpp = { kind: 'cpp', folder: '', place: '' };
	const NAME_RE = /^[A-Za-z_][A-Za-z0-9_]*$/;
	function cppFiles(name) {
		const n = name || '(名前)';
		const dir = cpp.place === '.' ? '' : cpp.place + String.fromCharCode(92); // 「¥」の区切り(テンプレート文字列の中なので文字コードで書く)
		if (cpp.kind === 'cpp') { return dir + n + '.cpp'; }
		if (cpp.kind === 'h') { return dir + n + '.h'; }
		return dir + n + '.h と ' + dir + n + '.cpp';
	}
	function updateCppPreview() { $('cf-file').textContent = '作る場所: ' + cppFiles($('cf-name').value.trim()); }
	function openCppForm(m) {
		cpp = { kind: m.kind, folder: m.folder, place: m.place };
		$('cf-title').textContent = m.title;
		$('cf-name').value = '';
		$('cf-error').textContent = '';
		showScreen('cpp-form');
		updateCppPreview();
		$('cf-name').focus();
	}
	function submitCpp() {
		const name = $('cf-name').value.trim();
		if (!NAME_RE.test(name)) { $('cf-error').textContent = '名前は英数字とアンダースコアだけで、先頭は英字か _ にしてください。'; return; }
		$('cf-error').textContent = '';
		showScreen('main');
		vscode.postMessage({ command: 'newCppFiles', args: { kind: cpp.kind, name, folder: cpp.folder } });
	}
	$('cf-name').addEventListener('input', updateCppPreview);
	$('cf-name').addEventListener('keydown', (e) => { if (e.key === 'Enter') { submitCpp(); } else if (e.key === 'Escape') { showScreen('main'); } });
	$('btn-cpp-create').addEventListener('click', submitCpp);
	$('btn-cpp-cancel').addEventListener('click', () => showScreen('main'));

	// --- 新しいシェーダー(欄の [作成]・右クリック・見出しのボタンから開く) ---------
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
			if (m.form === 'shader') { openShaderForm(); } else if (m.form === 'cpp') { openCppForm(m); } else { openTemplateForm(); }
			vscode.postMessage({ command: 'formOpened' });
		}
	});
	vscode.postMessage({ command: 'ready' });
</script>
</body>
</html>`;
	}
}

/** 欄に開くフォーム。cpp のときは種類と作る場所(folder は絶対パス、place はプロジェクトからの相対の表示用)。 */
export type FormRequest =
	| { form: 'shader' }
	| { form: 'template' }
	| { form: 'cpp'; kind: CppFileKind; title: string; folder: string; place: string };
