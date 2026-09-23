import * as path from 'path';
import * as vscode from 'vscode';
import {
	CppToolsApi,
	CustomConfigurationProvider,
	getCppToolsApi,
	SourceFileConfiguration,
	SourceFileConfigurationItem,
	Version,
	WorkspaceBrowseConfiguration,
} from 'vscode-cpptools';
import { getConfig } from '../env/environment';
import { intelliSenseState } from '../env/cpptools';
import { inspectSdk } from '../env/sdk';
import { detectVisualStudio } from '../env/vswhere';

const PROVIDER_ID = 'mahirocreative.dxlib-devenv';

/**
 * C/C++ 拡張へ IntelliSense の設定(cl.exe の場所、インクルードパス、定義)を渡す。
 * プロジェクトの c_cpp_properties.json には configurationProvider の指定だけを書く。
 */
export class DxLibConfigurationProvider implements CustomConfigurationProvider {
	readonly name = 'DxLib';
	readonly extensionId = PROVIDER_ID;
	private api: CppToolsApi | undefined;
	private cached: { config: SourceFileConfiguration; at: number } | undefined;

	async canProvideConfiguration(uri: vscode.Uri): Promise<boolean> {
		return /\.(c|cc|cpp|cxx|h|hpp|hxx|inl)$/i.test(uri.fsPath);
	}

	async provideConfigurations(uris: vscode.Uri[]): Promise<SourceFileConfigurationItem[]> {
		intelliSenseState.set('ready');
		const configuration = await this.baseConfiguration();
		return uris.map((uri) => ({ uri, configuration }));
	}

	async canProvideBrowseConfiguration(): Promise<boolean> {
		return true;
	}

	async provideBrowseConfiguration(): Promise<WorkspaceBrowseConfiguration | null> {
		const c = await this.baseConfiguration();
		return { browsePath: c.includePath, compilerPath: c.compilerPath, standard: c.standard };
	}

	async canProvideBrowseConfigurationsPerFolder(): Promise<boolean> {
		return false;
	}

	async provideFolderBrowseConfiguration(): Promise<WorkspaceBrowseConfiguration | null> {
		return null;
	}

	dispose(): void {
		this.api?.dispose();
	}

	/** SDK や VS の設定が変わったときに C/C++ 拡張へ再問い合わせを促す。 */
	invalidate(): void {
		this.cached = undefined;
		if (this.api) {
			this.api.didChangeCustomConfiguration(this);
			this.api.didChangeCustomBrowseConfiguration(this);
		}
	}

	async register(context: vscode.ExtensionContext): Promise<void> {
		const api = await getCppToolsApi(Version.latest);
		if (!api) {
			return;
		}
		this.api = api;
		api.registerCustomConfigurationProvider(this);
		api.notifyReady(this);
		context.subscriptions.push(api);
	}

	private async baseConfiguration(): Promise<SourceFileConfiguration> {
		if (this.cached && Date.now() - this.cached.at < 30_000) {
			return this.cached.config;
		}
		const vs = await detectVisualStudio();
		const sdk = inspectSdk(getConfig<string>('sdkPath', '') || undefined);
		const folder = vscode.workspace.workspaceFolders?.[0];
		const std = getConfig<string>('build.cppStandard', 'c++20', folder);
		const includePath: string[] = [];
		if (sdk?.ok) {
			includePath.push(sdk.path);
		}
		if (folder) {
			includePath.push(path.join(folder.uri.fsPath, 'src'));
		}
		const config: SourceFileConfiguration = {
			includePath,
			defines: ['_WINDOWS', 'WIN32', '_DEBUG', '_UNICODE=0'],
			intelliSenseMode: 'windows-msvc-x64',
			standard: std === 'c++latest' ? 'c++23' : (std as SourceFileConfiguration['standard']),
			compilerPath: vs.clPath,
		};
		this.cached = { config, at: Date.now() };
		return config;
	}
}
