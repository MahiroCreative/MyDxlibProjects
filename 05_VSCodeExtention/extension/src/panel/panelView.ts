import * as fs from 'fs';
import * as vscode from 'vscode';
import { collectEnvironment, EnvironmentStatus, getConfig } from '../env/environment';
import { CreateProjectArgs, defaultCreateLocation } from '../project/createProject';
import { SaveTemplateArgs } from '../project/saveAsTemplate';
import { listTemplates, TemplateInfo } from '../project/templates';
import { NewShaderArgs, SHADER_TEMPLATES } from '../shader/compileShaders';

/** どのフォームを開くか。'create' | 'shader' | 'template' */
type FormName = 'create' | 'shader' | 'template';

/** Webview へ渡す状態(JSON にできる形)。 */
interface PanelState {
	vs: { state: string; label: string };
	sdk: { ok: boolean; label: string; path: string; missing: string[] };
	cpptools: string;
	templatesPath: string;
	project?: { name: string; path: string };
	isDxLibProject: boolean;
	templates: { id: string; name: string; description: string; builtin: boolean }[];
	shaderKinds: { id: string; label: string; description: string; suffix: string }[];
	defaultLocation: string;
}

type WebviewMessage =
	| { command: 'refresh' }
	| { command: 'browseLocation' }
	| { command: 'create'; args: CreateProjectArgs }
	| { command: 'newShaderFile'; args: NewShaderArgs }
	| { command: 'saveAsTemplate'; args: SaveTemplateArgs }
	| { command: string };

function toState(env: EnvironmentStatus, templates: TemplateInfo[], defaultLocation: string): PanelState {
	let vsLabel: string;
	switch (env.vs.state) {
		case 'ok':
			vsLabel = `${env.vs.displayName ?? 'Visual Studio'}`;
			break;
		case 'noWorkload':
			vsLabel = `${env.vs.displayName ?? 'Visual Studio'} に「C++ によるデスクトップ開発」が未インストール`;
			break;
		default:
			vsLabel = 'Visual Studio が見つかりません';
	}
	let sdkLabel: string;
	if (!env.sdkPath) {
		sdkLabel = 'DxLib SDK 未設定';
	} else if (env.sdk?.ok) {
		sdkLabel = `DxLib SDK ${env.sdk.version}`;
	} else if (env.sdk?.version) {
		sdkLabel = `DxLib SDK ${env.sdk.version}(ファイル不足: ${env.sdk.missing.slice(0, 3).join(', ')}…)`;
	} else {
		sdkLabel = 'DxLib SDK のフォルダが正しくありません(DxLib.h が見つかりません)';
	}
	return {
		vs: { state: env.vs.state, label: vsLabel },
		sdk: { ok: !!env.sdk?.ok, label: sdkLabel, path: env.sdkPath, missing: env.sdk?.missing ?? [] },
		cpptools: env.cpptools,
		templatesPath: env.templatesPath,
		project: env.project,
		isDxLibProject: env.isDxLibProject,
		templates: templates.map((t) => ({ id: t.id, name: t.name, description: t.description, builtin: t.builtin })),
		shaderKinds: SHADER_TEMPLATES.map((t) => ({ id: t.id, label: t.label, description: t.description, suffix: t.suffix })),
		defaultLocation,
	};
}

export class DxLibPanelProvider implements vscode.WebviewViewProvider {
	static readonly viewType = 'dxlib.panel';
	private view: vscode.WebviewView | undefined;

	constructor(private readonly context: vscode.ExtensionContext) {}

	resolveWebviewView(view: vscode.WebviewView): void {
		this.view = view;
		view.webview.options = { enableScripts: true, localResourceRoots: [this.context.extensionUri] };
		view.webview.html = this.html(view.webview);
		view.webview.onDidReceiveMessage((m: WebviewMessage) => void this.onMessage(m));
		view.onDidChangeVisibility(() => {
			if (view.visible) {
				void this.refresh();
			}
		});
		void this.refresh();
	}

	async refresh(): Promise<void> {
		if (!this.view) {
			return;
		}
		if (!vscode.workspace.isTrusted) {
			// 制限モード: 環境の検出(外部プロセスの起動を含む)もせず、信頼の案内だけを出す
			void this.view.webview.postMessage({ type: 'restricted' });
			return;
		}
		const env = await collectEnvironment();
		const templates = listTemplates(this.context, getConfig<string>('templatesPath', ''));
		void this.view.webview.postMessage({ type: 'status', state: toState(env, templates, defaultCreateLocation(this.context)) });
	}

	private async openForm(form: FormName, location?: string): Promise<void> {
		await vscode.commands.executeCommand('workbench.view.extension.dxlib');
		await this.refresh();
		void this.view?.webview.postMessage({ type: 'openForm', form, location });
	}

	/** 作成フォームを開く(作成先の初期値を指定できる)。 */
	async showCreateForm(location?: string): Promise<void> {
		await this.openForm('create', location);
	}

	/** 「新しいシェーダー」フォームを開く。 */
	async showNewShaderForm(): Promise<void> {
		await this.openForm('shader');
	}

	/** 「テンプレートとして保存」フォームを開く。 */
	async showSaveTemplateForm(): Promise<void> {
		await this.openForm('template');
	}

	private async onMessage(m: WebviewMessage): Promise<void> {
		switch (m.command) {
			case 'refresh':
				await this.refresh();
				return;
			case 'openCreateForm':
				// 最新の状態(前回の作成先など)を取り直してからフォームを開く
				await this.showCreateForm();
				return;
			case 'browseLocation': {
				// 入力欄の場所(無ければ前回の作成先)からフォルダ選択を始める
				const current = (m as { current?: string }).current;
				const start = current && fs.existsSync(current) ? current : defaultCreateLocation(this.context);
				const picked = await vscode.window.showOpenDialog({ canSelectFolders: true, canSelectFiles: false, canSelectMany: false, openLabel: '作成先にする', defaultUri: vscode.Uri.file(start) });
				if (picked?.[0]) {
					void this.view?.webview.postMessage({ type: 'location', path: picked[0].fsPath });
				}
				return;
			}
			case 'create':
				await vscode.commands.executeCommand('dxlib.createProject', (m as { args: CreateProjectArgs }).args);
				return;
			case 'newShaderFile':
				await vscode.commands.executeCommand('dxlib.newShaderFile', (m as { args: NewShaderArgs }).args);
				return;
			case 'saveAsTemplate':
				await vscode.commands.executeCommand('dxlib.saveAsTemplate', (m as { args: SaveTemplateArgs }).args);
				return;
			default:
				// それ以外は同名の dxlib.* コマンドを呼ぶ(build / run / debug / selectSdk など)
				await vscode.commands.executeCommand(`dxlib.${m.command}`);
		}
	}

	private html(webview: vscode.Webview): string {
		const nonce = Array.from({ length: 16 }, () => Math.floor(Math.random() * 36).toString(36)).join('');
		return /* html */ `<!DOCTYPE html>
<html lang="ja">
<head>
<meta charset="UTF-8">
<meta http-equiv="Content-Security-Policy" content="default-src 'none'; style-src 'unsafe-inline' ${webview.cspSource}; script-src 'nonce-${nonce}';">
<style>
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
	button.secondary { background: var(--vscode-button-secondaryBackground); color: var(--vscode-button-secondaryForeground); }
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
	#form, #shader-form, #template-form, #restricted { display: none; }
	#restricted p { line-height: 1.5; margin: 6px 0; }
	.hidden { display: none !important; }
</style>
</head>
<body>
	<div id="restricted">
		<h2>制限モード</h2>
		<p>このフォルダは「制限モード」で開かれています。VSCode がまだこのフォルダを信頼していないため、ビルド・実行・コード補完などは止まっています。</p>
		<p>自分で作ったプロジェクトなら、下のボタンから「信頼する」を選んでください。開き直さなくても、そのまま使えるようになります。</p>
		<button class="big" data-cmd="manageTrust">このフォルダを信頼する</button>
		<div class="hint">ボタンを押すと開く画面の「Trust」(信頼する)を押します。プロジェクトを作る場所(親フォルダ)を、その画面の下の「Add Folder」(フォルダーの追加)で信頼済みにしておくと、同じ場所に作るプロジェクトでは次から聞かれません。</div>
	</div>

	<div id="main">
		<h2>環境</h2>
		<div class="row"><span id="vs-mark"></span><span class="text" id="vs-label">確認中…</span><span id="vs-actions"></span></div>
		<div class="row"><span id="sdk-mark"></span><span class="text"><span id="sdk-label">確認中…</span><span class="path" id="sdk-path"></span></span><button class="secondary" data-cmd="selectSdk">変更</button></div>
		<div class="row"><span id="cpp-mark"></span><span class="text" id="cpp-label">C/C++ 拡張</span><span id="cpp-actions"></span></div>
		<div class="row"><span>📁</span><span class="text"><span>テンプレート</span><span class="path" id="tpl-path"></span></span><button class="secondary" data-cmd="selectTemplatesDir">変更</button></div>
		<div class="actions"><button class="secondary" data-cmd="refresh">再チェック</button><button class="secondary" data-cmd="openSetupGuide">手順を見る</button></div>

		<h2>プロジェクト</h2>
		<button class="big" id="btn-open-form">新規プロジェクト作成</button>
		<div class="error hidden" id="create-blocked">DxLib SDK が正しく設定されるまで、プロジェクトは作成できません。上の SDK の「変更」から、正しいフォルダを指定してください。</div>
		<div id="project-section">
			<div class="row"><span class="text">現在のプロジェクト: <b id="project-name"></b></span></div>
			<div class="actions">
				<button data-cmd="build">ビルド</button>
				<button data-cmd="run">実行</button>
				<button data-cmd="debug">デバッグ実行</button>
				<button class="secondary" data-cmd="compileShaders">シェーダーをコンパイル</button>
				<button class="secondary" id="btn-open-shader-form">新しいシェーダー</button>
				<button class="secondary" id="btn-open-template-form">テンプレートとして保存</button>
			</div>
		</div>
		<div id="no-project" class="hidden"><span class="wait" id="no-project-text">プロジェクトのフォルダが開かれていません。</span></div>
	</div>

	<div id="form">
		<h2>新規プロジェクト作成</h2>
		<label for="f-name">プロジェクト名(英数字と _)</label>
		<input type="text" id="f-name" value="MyGame">
		<label for="f-location">作成先</label>
		<div class="row"><input type="text" id="f-location"><button class="secondary" id="btn-browse">参照</button></div>
		<label>テンプレート</label>
		<div id="f-templates"></div>
		<div class="error" id="f-error"></div>
		<div class="actions"><button id="btn-create">作成</button><button class="secondary" id="btn-cancel">キャンセル</button></div>
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
	let state = null;

	document.querySelectorAll('[data-cmd]').forEach((b) => b.addEventListener('click', () => vscode.postMessage({ command: b.dataset.cmd })));

	function showScreen(name) {
		for (const id of ['main', 'form', 'shader-form', 'template-form', 'restricted']) { $(id).style.display = (id === name) ? 'block' : 'none'; }
	}

	// --- 新規プロジェクト作成 -------------------------------------------------
	$('btn-open-form').addEventListener('click', () => vscode.postMessage({ command: 'openCreateForm' }));
	$('btn-cancel').addEventListener('click', () => showScreen('main'));
	$('btn-browse').addEventListener('click', () => vscode.postMessage({ command: 'browseLocation', current: $('f-location').value.trim() }));
	$('btn-create').addEventListener('click', () => {
		const name = $('f-name').value.trim();
		const location = $('f-location').value.trim();
		const picked = document.querySelector('input[name=tpl]:checked');
		if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(name)) { $('f-error').textContent = 'プロジェクト名は英数字とアンダースコアだけで、先頭は英字か _ にしてください。'; return; }
		if (!location) { $('f-error').textContent = '作成先を指定してください。'; return; }
		if (!picked) { $('f-error').textContent = 'テンプレートを選んでください。'; return; }
		$('f-error').textContent = '';
		vscode.postMessage({ command: 'create', args: { name, location, templateId: picked.value } });
	});

	function openCreateForm(location) {
		showScreen('form');
		// 開くたびに初期値(前回の作成先)を入れ直す。右クリックから開いたときはそのフォルダ。
		if (location) { $('f-location').value = location; }
		else if (state) { $('f-location').value = state.defaultLocation; }
		renderTemplates();
	}

	function renderTemplates() {
		const box = $('f-templates'); box.innerHTML = '';
		if (!state) return;
		state.templates.forEach((t, i) => {
			const row = document.createElement('label'); row.className = 'tpl';
			const r = document.createElement('input'); r.type = 'radio'; r.name = 'tpl'; r.value = t.id; if (i === 0) r.checked = true;
			const txt = document.createElement('span'); txt.textContent = t.name;
			const small = document.createElement('small'); small.textContent = t.description;
			txt.appendChild(small);
			row.appendChild(r); row.appendChild(txt); box.appendChild(row);
		});
		if (state.templates.length === 0) { box.textContent = 'テンプレートがありません。'; }
	}

	// --- 新しいシェーダー ------------------------------------------------------
	$('btn-open-shader-form').addEventListener('click', () => openShaderForm());
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
		if (!state) return;
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
		if (!state) return;
		const picked = document.querySelector('input[name=sfkind]:checked');
		const kind = state.shaderKinds.find((k) => picked && k.id === picked.value);
		const name = $('sf-name').value.trim() || '(名前)';
		$('sf-filename').textContent = 'ファイル名: ' + name + (kind ? kind.suffix : '') + '.hlsl';
	}
	$('sf-name').addEventListener('input', updateShaderFilenamePreview);

	// --- テンプレートとして保存 -------------------------------------------------
	$('btn-open-template-form').addEventListener('click', () => openTemplateForm());
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
		if (state && state.project && !$('tf-name').value) { $('tf-name').value = state.project.name; }
	}

	function mark(el, cls, text) { el.className = cls; el.textContent = text; }
	function buttons(container, list) {
		container.innerHTML = '';
		for (const [label, cmd] of list) {
			const b = document.createElement('button'); b.className = 'secondary'; b.textContent = label; b.style.marginLeft = '4px';
			b.addEventListener('click', () => vscode.postMessage({ command: cmd }));
			container.appendChild(b);
		}
	}

	function render() {
		const s = state;
		if (s.vs.state === 'ok') { mark($('vs-mark'), 'ok', '✓'); buttons($('vs-actions'), []); }
		else if (s.vs.state === 'noWorkload') { mark($('vs-mark'), 'ng', '✗'); buttons($('vs-actions'), [['ワークロードを追加', 'addCppWorkload']]); }
		else { mark($('vs-mark'), 'ng', '✗'); buttons($('vs-actions'), [['ダウンロードページ', 'openVsDownload']]); }
		$('vs-label').textContent = s.vs.label;

		mark($('sdk-mark'), s.sdk.ok ? 'ok' : 'ng', s.sdk.ok ? '✓' : '✗');
		$('sdk-label').textContent = s.sdk.label;
		$('sdk-path').textContent = s.sdk.path || '';
		$('sdk-path').title = s.sdk.path || '';

		const cpp = { missing: ['ng', '✗', 'C/C++ 拡張が無効か未インストールです'], preparing: ['wait', '…', 'C/C++ 拡張: 準備中(.cpp を開くと完了)'], ready: ['ok', '✓', 'C/C++ 拡張'], unresponsive: ['warn', '!', 'C/C++ 拡張が応答しません'] }[s.cpptools] || ['wait', '?', 'C/C++ 拡張'];
		mark($('cpp-mark'), cpp[0], cpp[1]); $('cpp-label').textContent = cpp[2];
		buttons($('cpp-actions'), s.cpptools === 'missing' ? [['拡張機能を開く', 'openCpptools']] : []);

		$('tpl-path').textContent = s.templatesPath || '(未設定。同梱テンプレートのみ)';
		$('tpl-path').title = s.templatesPath || '';

		if (s.project && s.isDxLibProject) {
			$('project-section').classList.remove('hidden');
			$('no-project').classList.add('hidden');
			$('project-name').textContent = s.project.name;
		} else {
			$('project-section').classList.add('hidden');
			$('no-project').classList.remove('hidden');
			$('no-project-text').textContent = s.project
				? 'このフォルダは DxLib プロジェクトではありません。上の「新規プロジェクト作成」で作るか、既存のプロジェクトのフォルダを開いてください。'
				: 'プロジェクトのフォルダが開かれていません。';
		}
		// SDK が正しくない間はプロジェクトを作成できない(ボタンを無効にして理由を出す。開いている作成フォームは閉じる)
		$('btn-open-form').disabled = !s.sdk.ok;
		$('create-blocked').classList.toggle('hidden', s.sdk.ok);
		if (!s.sdk.ok && $('form').style.display === 'block') { showScreen('main'); }
		if ($('form').style.display === 'block') { renderTemplates(); }
		if ($('shader-form').style.display === 'block') { renderShaderKinds(); updateShaderFilenamePreview(); }
	}

	window.addEventListener('message', (e) => {
		const m = e.data;
		if (m.type === 'restricted') {
			showScreen('restricted');
		} else if (m.type === 'status') {
			// 信頼された直後は制限モードの画面から通常の画面に戻す
			if ($('restricted').style.display === 'block') { showScreen('main'); }
			state = m.state;
			render();
		} else if (m.type === 'openForm') {
			if (m.form === 'shader') { openShaderForm(); }
			else if (m.form === 'template') { openTemplateForm(); }
			else { openCreateForm(m.location); }
		} else if (m.type === 'location') {
			$('f-location').value = m.path;
		}
	});
	vscode.postMessage({ command: 'refresh' });
</script>
</body>
</html>`;
	}
}
