import * as fs from 'fs';
import * as vscode from 'vscode';
import { collectEnvironment, EnvironmentStatus } from '../env/environment';
import { CreateProjectArgs, defaultCreateLocation } from '../project/createProject';
import { addRecentTemplate, listTemplates, readTemplateZip, TemplateInfo } from '../project/templates';
import { BASE_CSS, makeNonce } from './webviewCommon';

/** どのフォームを開くか。プロジェクトの操作のフォームはエクスプローラーの「DxLib」欄(projectView.ts)。 */
type FormName = 'create';

/** Webview へ渡す状態(JSON にできる形)。 */
interface PanelState {
	vs: {
		state: string;
		label: string;
		/** 選んでいた Visual Studio が見つからないときの説明(DESIGN.md 4 章)。 */
		note: string;
		/** C++ ワークロードの入った Visual Studio すべて。2 つ以上なら [変更] を出す。 */
		installs: { name: string; version: string; path: string }[];
		current: string;
	};
	sdk: { ok: boolean; label: string; path: string; missing: string[] };
	cpptools: string;
	project?: { name: string; path: string };
	isDxLibProject: boolean;
	/** Visual Studio で作ったプロジェクトの .vcxproj の名前(DESIGN.md 6.1 章)。 */
	vsProject?: string;
	templates: { id: string; name: string; description: string; builtin: boolean; source: string }[];
	defaultLocation: string;
}

type WebviewMessage =
	| { command: 'refresh' }
	| { command: 'browseLocation' }
	| { command: 'create'; args: CreateProjectArgs }
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
		vs: {
			state: env.vs.state,
			label: vsLabel,
			note: env.vs.selectionMissing ? `選んでいた Visual Studio が見つからないので、${env.vs.displayName ?? 'いちばん新しいもの'} を使っています` : '',
			installs: (env.vs.installs ?? []).map((i) => ({ name: i.displayName, version: i.version, path: i.installationPath })),
			current: env.vs.installationPath ?? '',
		},
		sdk: { ok: !!env.sdk?.ok, label: sdkLabel, path: env.sdkPath, missing: env.sdk?.missing ?? [] },
		cpptools: env.cpptools,
		project: env.project,
		isDxLibProject: env.isDxLibProject,
		vsProject: env.vsProject,
		templates: templates.map((t) => ({ id: t.id, name: t.name, description: t.description, builtin: t.builtin, source: t.source })),
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
		const templates = listTemplates(this.context);
		void this.view.webview.postMessage({ type: 'status', state: toState(env, templates, defaultCreateLocation(this.context)) });
	}

	private async openForm(form: FormName, location?: string): Promise<void> {
		await vscode.commands.executeCommand('workbench.view.extension.dxlib');
		await this.refresh();
		void this.view?.webview.postMessage({ type: 'openForm', form, location });
	}

	/** テンプレートファイルを確かめて「最近使ったテンプレート」に入れ、作成フォームで選んだ状態にする(検証からも呼ぶ)。 */
	async useTemplateZip(file: string): Promise<boolean> {
		try {
			readTemplateZip(file);
		} catch (e) {
			void vscode.window.showErrorMessage(e instanceof Error ? e.message : String(e));
			return false;
		}
		addRecentTemplate(this.context, file);
		await this.refresh();
		void this.view?.webview.postMessage({ type: 'templatePicked', id: `file:${file}` });
		return true;
	}

	/** 作成フォームを開く(作成先の初期値を指定できる)。 */
	async showCreateForm(location?: string): Promise<void> {
		await this.openForm('create', location);
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
			case 'pickTemplate': {
				// テンプレートファイルを選ぶ(Windows のファイルを開くダイアログ。手作りの .zip も可)。中身を確かめてから一覧に足して選ぶ
				const picked = await vscode.window.showOpenDialog({
					title: 'テンプレートファイルを選ぶ',
					canSelectFiles: true,
					canSelectFolders: false,
					canSelectMany: false,
					openLabel: 'このテンプレートを使う',
					filters: { 'DxLib テンプレート': ['dxtemplate', 'zip'] },
				});
				if (!picked?.[0]) {
					return;
				}
				await this.useTemplateZip(picked[0].fsPath);
				return;
			}
			case 'selectVisualStudio':
				await vscode.commands.executeCommand('dxlib.selectVisualStudio', (m as { path?: string }).path);
				return;
			case 'create':
				await vscode.commands.executeCommand('dxlib.createProject', (m as { args: CreateProjectArgs }).args);
				return;
			default:
				// それ以外は同名の dxlib.* コマンドを呼ぶ(build / run / debug / selectSdk など)
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
	#form, #restricted, #vs-form { display: none; }
	#restricted p { line-height: 1.5; margin: 6px 0; }
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
		<div class="row"><span id="vs-mark"></span><span class="text"><span id="vs-label">確認中…</span><span class="path" id="vs-note"></span></span><span id="vs-actions"></span></div>
		<div class="row"><span id="sdk-mark"></span><span class="text"><span id="sdk-label">確認中…</span><span class="path" id="sdk-path"></span></span><button class="secondary" data-cmd="selectSdk">変更</button></div>
		<div class="row"><span id="cpp-mark"></span><span class="text" id="cpp-label">C/C++ 拡張</span><span id="cpp-actions"></span></div>
		<div class="actions"><button class="secondary" data-cmd="refresh">再チェック</button><button class="secondary" data-cmd="openSetupGuide">手順を見る</button></div>

		<h2>プロジェクト</h2>
		<button class="big" id="btn-open-form">新規プロジェクト作成</button>
		<div class="error hidden" id="create-blocked">DxLib SDK が正しく設定されるまで、プロジェクトは作成できません。上の SDK の「変更」から、正しいフォルダを指定してください。</div>
		<div id="project-section">
			<div class="row"><span class="text">現在のプロジェクト: <b id="project-name"></b></span></div>
			<div class="hint">ビルド・実行・デバッグはエディタ右上のボタン、ファイルの作成・シェーダー・テンプレートとして保存はエクスプローラーの「DxLib」欄にあります。</div>
			<div class="actions"><button class="secondary" data-cmd="showProjectView">エクスプローラーの DxLib 欄を開く</button></div>
		</div>
		<div id="no-project" class="hidden"><span class="wait" id="no-project-text">プロジェクトのフォルダが開かれていません。</span></div>
		<div id="vs-project" class="hidden">
			<div>Visual Studio のプロジェクトです(<b id="vs-project-name"></b>)。このまま VSCode でビルド・実行・デバッグできるようにします。</div>
			<div class="hint">.vcxproj はそのまま使います(Visual Studio でも引き続き開けます)。</div>
			<button class="big" data-cmd="adoptVsProject">DxLib 拡張で使えるようにする</button>
		</div>
	</div>

	<div id="vs-form">
		<h2>使う Visual Studio を選ぶ</h2>
		<div class="hint">この PC に入っている Visual Studio(C++ によるデスクトップ開発あり)です。ビルドと補完に、選んだものを使います。</div>
		<div id="vsf-list"></div>
		<div class="hint">Visual Studio で作ったプロジェクトは、作ったときの版を選んでください(ほかの版ではビルドできないことがあります)。</div>
		<div class="actions"><button id="btn-vs-select">決定</button><button class="secondary" id="btn-vs-cancel">キャンセル</button></div>
	</div>

	<div id="form">
		<h2>新規プロジェクト作成</h2>
		<label for="f-name">プロジェクト名(英数字と _)</label>
		<input type="text" id="f-name" value="MyGame">
		<label for="f-location">作成先</label>
		<div class="row"><input type="text" id="f-location"><button class="secondary" id="btn-browse">参照</button></div>
		<label>テンプレート</label>
		<div id="f-templates"></div>
		<div class="actions"><button class="secondary" id="btn-pick-template" title="授業などで配られたテンプレートファイル(.dxtemplate)を選ぶ">テンプレートファイル (.dxtemplate) を選ぶ...</button></div>
		<div class="error" id="f-error"></div>
		<div class="actions"><button id="btn-create">作成</button><button class="secondary" id="btn-cancel">キャンセル</button></div>
	</div>

<script nonce="${nonce}">
	const vscode = acquireVsCodeApi();
	const $ = (id) => document.getElementById(id);
	let state = null;
	let selectedTemplate = '';

	document.querySelectorAll('[data-cmd]').forEach((b) => b.addEventListener('click', () => vscode.postMessage({ command: b.dataset.cmd })));

	function showScreen(name) {
		for (const id of ['main', 'form', 'restricted', 'vs-form']) { $(id).style.display = (id === name) ? 'block' : 'none'; }
	}

	// --- 新規プロジェクト作成 -------------------------------------------------
	$('btn-open-form').addEventListener('click', () => vscode.postMessage({ command: 'openCreateForm' }));
	$('btn-cancel').addEventListener('click', () => showScreen('main'));
	$('btn-pick-template').addEventListener('click', () => vscode.postMessage({ command: 'pickTemplate' }));
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
		// 選んでいたテンプレートは、一覧を作り直しても選んだままにする(無くなっていたら先頭)
		if (!state.templates.some((t) => t.id === selectedTemplate)) { selectedTemplate = state.templates.length ? state.templates[0].id : ''; }
		state.templates.forEach((t) => {
			const row = document.createElement('label'); row.className = 'tpl';
			const r = document.createElement('input'); r.type = 'radio'; r.name = 'tpl'; r.value = t.id; r.checked = (t.id === selectedTemplate);
			r.addEventListener('change', () => { selectedTemplate = t.id; });
			const txt = document.createElement('span'); txt.textContent = t.builtin ? t.name + '(同梱)' : t.name;
			const small = document.createElement('small'); small.textContent = t.description;
			txt.appendChild(small);
			if (!t.builtin) {
				// テンプレートファイルは場所も出す(同じ名前のテンプレートを見分けられるように)
				const where = document.createElement('small'); where.textContent = t.source; where.title = t.source;
				txt.appendChild(where);
			}
			row.appendChild(r); row.appendChild(txt); box.appendChild(row);
		});
		if (state.templates.length === 0) { box.textContent = 'テンプレートがありません。'; }
	}

	function mark(el, cls, text) { el.className = cls; el.textContent = text; }
	function buttons(container, list) {
		container.innerHTML = '';
		for (const [label, cmd] of list) {
			const b = document.createElement('button'); b.className = 'secondary'; b.textContent = label; b.style.marginLeft = '4px';
			// '#vsSelect' はパネルの中で選択のフォームを開く(拡張機能のコマンドではない)
			b.addEventListener('click', () => (cmd === '#vsSelect' ? openVsForm() : vscode.postMessage({ command: cmd })));
			container.appendChild(b);
		}
	}

	// --- 使う Visual Studio を選ぶ(DESIGN.md 4 章) ---------------------------------
	function openVsForm() {
		if (!state) return;
		const box = $('vsf-list'); box.innerHTML = '';
		state.vs.installs.forEach((i) => {
			const row = document.createElement('label'); row.className = 'tpl';
			const r = document.createElement('input'); r.type = 'radio'; r.name = 'vsf'; r.value = i.path;
			r.checked = i.path.toLowerCase() === state.vs.current.toLowerCase();
			const txt = document.createElement('span'); txt.textContent = i.name;
			const small = document.createElement('small'); small.textContent = i.version + '  ' + i.path; small.title = i.path;
			txt.appendChild(small);
			row.appendChild(r); row.appendChild(txt); box.appendChild(row);
		});
		showScreen('vs-form');
	}
	$('btn-vs-cancel').addEventListener('click', () => showScreen('main'));
	$('btn-vs-select').addEventListener('click', () => {
		const picked = document.querySelector('input[name=vsf]:checked');
		showScreen('main');
		if (picked) { vscode.postMessage({ command: 'selectVisualStudio', path: picked.value }); }
	});

	function render() {
		const s = state;
		// Visual Studio が 2 つ以上あるときだけ [変更](DESIGN.md 4 章)
		const vsChange = s.vs.installs.length > 1 ? [['変更', '#vsSelect']] : [];
		if (s.vs.state === 'ok') { mark($('vs-mark'), 'ok', '✓'); buttons($('vs-actions'), vsChange); }
		else if (s.vs.state === 'noWorkload') { mark($('vs-mark'), 'ng', '✗'); buttons($('vs-actions'), [['ワークロードを追加', 'addCppWorkload'], ...vsChange]); }
		else { mark($('vs-mark'), 'ng', '✗'); buttons($('vs-actions'), [['ダウンロードページ', 'openVsDownload']]); }
		$('vs-label').textContent = s.vs.label;
		$('vs-note').textContent = s.vs.note;

		mark($('sdk-mark'), s.sdk.ok ? 'ok' : 'ng', s.sdk.ok ? '✓' : '✗');
		$('sdk-label').textContent = s.sdk.label;
		$('sdk-path').textContent = s.sdk.path || '';
		$('sdk-path').title = s.sdk.path || '';

		const cpp = { missing: ['ng', '✗', 'C/C++ 拡張が無効か未インストールです'], preparing: ['wait', '…', 'C/C++ 拡張: 準備中(.cpp を開くと完了)'], ready: ['ok', '✓', 'C/C++ 拡張'], unresponsive: ['warn', '!', 'C/C++ 拡張が応答しません'] }[s.cpptools] || ['wait', '?', 'C/C++ 拡張'];
		mark($('cpp-mark'), cpp[0], cpp[1]); $('cpp-label').textContent = cpp[2];
		buttons($('cpp-actions'), s.cpptools === 'missing' ? [['拡張機能を開く', 'openCpptools']] : []);


		if (s.project && s.isDxLibProject) {
			$('project-section').classList.remove('hidden');
			$('no-project').classList.add('hidden');
			$('vs-project').classList.add('hidden');
			$('project-name').textContent = s.project.name;
		} else if (s.vsProject) {
			// Visual Studio で作ったプロジェクト: 使えるようにするボタンを出す(DESIGN.md 6.1 章)
			$('project-section').classList.add('hidden');
			$('no-project').classList.add('hidden');
			$('vs-project').classList.remove('hidden');
			$('vs-project-name').textContent = s.vsProject;
		} else {
			$('project-section').classList.add('hidden');
			$('vs-project').classList.add('hidden');
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
			openCreateForm(m.location);
		} else if (m.type === 'location') {
			$('f-location').value = m.path;
		} else if (m.type === 'templatePicked') {
			// 選んだテンプレートファイルを選んだ状態にする(一覧は直前の status で更新済み)
			selectedTemplate = m.id;
			if ($('form').style.display === 'block') { renderTemplates(); }
		}
	});
	vscode.postMessage({ command: 'refresh' });
</script>
</body>
</html>`;
	}
}
