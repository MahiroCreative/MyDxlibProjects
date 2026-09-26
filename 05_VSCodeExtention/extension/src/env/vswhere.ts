import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import * as vscode from 'vscode';
import { run } from '../util/exec';

export type VsState = 'notFound' | 'noWorkload' | 'ok';

export interface VsInfo {
	state: VsState;
	displayName?: string;
	version?: string;
	installationPath?: string;
	/** VC\Auxiliary\Build\vcvarsall.bat */
	vcvarsall?: string;
	/** IntelliSense 用の cl.exe(Hostx64\x64)。 */
	clPath?: string;
	/** ビルドに使う MSBuild.exe(DESIGN.md 6 章)。 */
	msbuild?: string;
	/** C++ ワークロードの入った Visual Studio すべて(新しい順)。2 つ以上なら DxLib パネルで選べる(DESIGN.md 4 章)。 */
	installs?: VsInstall[];
	/** 設定で選んでいた Visual Studio が見つからず、いちばん新しいものを使っている。 */
	selectionMissing?: boolean;
}

export interface VsInstall {
	displayName: string;
	version: string;
	installationPath: string;
}

/** C++ によるデスクトップ開発ワークロードに含まれる MSVC ツールセットのコンポーネント ID。 */
const VC_TOOLS_COMPONENT = 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64';

/** vswhere からワークロードを追加するときに渡す ID。 */
export const NATIVE_DESKTOP_WORKLOAD = 'Microsoft.VisualStudio.Workload.NativeDesktop';

/**
 * Visual Studio Installer(setup.exe)に C++ ワークロードを追加させる引数。
 * --passive: 進行状況だけ表示し、操作を求めない(生徒は UAC の「はい」を押すだけ)。
 * --norestart: 完了後に勝手に PC を再起動しない。
 */
export function workloadInstallArgs(installationPath: string): string[] {
	return ['modify', '--installPath', installationPath, '--add', NATIVE_DESKTOP_WORKLOAD, '--includeRecommended', '--passive', '--norestart'];
}

function installerDir(): string {
	return path.join(process.env['ProgramFiles(x86)'] ?? 'C:\\Program Files (x86)', 'Microsoft Visual Studio', 'Installer');
}

export function vswherePath(): string | undefined {
	const p = path.join(installerDir(), 'vswhere.exe');
	return fs.existsSync(p) ? p : undefined;
}

export function installerSetupPath(): string | undefined {
	const p = path.join(installerDir(), 'setup.exe');
	return fs.existsSync(p) ? p : undefined;
}

interface VswhereEntry {
	displayName?: string;
	installationVersion?: string;
	installationPath?: string;
}

async function query(extraArgs: string[]): Promise<VswhereEntry[]> {
	const vw = vswherePath();
	if (!vw) {
		return [];
	}
	const r = await run(vw, ['-products', '*', '-prerelease', '-format', 'json', '-utf8', ...extraArgs]);
	try {
		const parsed = JSON.parse(r.stdout || '[]');
		return Array.isArray(parsed) ? parsed : [];
	} catch {
		return [];
	}
}

/** Visual Studio 2019 以降の MSBuild(<VS>\MSBuild\Current\Bin\MSBuild.exe)。 */
function findMsbuild(installationPath: string): string | undefined {
	const p = path.join(installationPath, 'MSBuild', 'Current', 'Bin', 'MSBuild.exe');
	return fs.existsSync(p) ? p : undefined;
}

function findCl(installationPath: string): string | undefined {
	const msvc = path.join(installationPath, 'VC', 'Tools', 'MSVC');
	if (!fs.existsSync(msvc)) {
		return undefined;
	}
	const versions = fs.readdirSync(msvc).sort().reverse();
	for (const v of versions) {
		const cl = path.join(msvc, v, 'bin', 'Hostx64', 'x64', 'cl.exe');
		if (fs.existsSync(cl)) {
			return cl;
		}
	}
	return undefined;
}

/** Visual Studio と C++ ワークロードの有無を調べる。 */
export async function detectVisualStudio(): Promise<VsInfo> {
	const vs = await detectVisualStudioActual();
	// 検証用: ワークロードが入っている PC で「ワークロードを追加」の流れを実際に通すため、未導入として扱う(DESIGN.md 4 章)
	if (process.env.DXLIB_TEST_SIMULATE_NO_WORKLOAD === '1' && vs.state === 'ok') {
		return { state: 'noWorkload', displayName: vs.displayName, version: vs.version, installationPath: vs.installationPath };
	}
	return vs;
}

/** 版の新しい順に並べるための比較(18.10.x > 17.14.x)。 */
function compareVersion(a: string, b: string): number {
	const pa = a.split('.').map((x) => Number(x) || 0);
	const pb = b.split('.').map((x) => Number(x) || 0);
	for (let i = 0; i < Math.max(pa.length, pb.length); i++) {
		if ((pa[i] ?? 0) !== (pb[i] ?? 0)) {
			return (pb[i] ?? 0) - (pa[i] ?? 0);
		}
	}
	return 0;
}

/** 検証用(DESIGN.md 4 章): 実在しない Visual Studio をもう 1 つ見つかったことにする。 */
export const FAKE_VS_PATH = path.join(os.tmpdir(), 'dxlib-test-fake-vs');

async function detectVisualStudioActual(): Promise<VsInfo> {
	const found = await query(['-all', '-requires', VC_TOOLS_COMPONENT]);
	const installs: VsInstall[] = found
		.filter((e) => e.installationPath)
		.map((e) => ({ displayName: e.displayName ?? 'Visual Studio', version: e.installationVersion ?? '', installationPath: e.installationPath as string }));
	if (process.env.DXLIB_TEST_EXTRA_VS === '1' && installs.length > 0) {
		installs.push({ displayName: 'Visual Studio Community 2022(検証用)', version: '17.99.0.0', installationPath: FAKE_VS_PATH });
	}
	installs.sort((a, b) => compareVersion(a.version, b.version));
	if (installs.length > 0) {
		// 使うのは DxLib パネルで選んだもの(設定 dxlib.visualStudioPath)。未設定・見つからなければいちばん新しいもの
		const wanted = vscode.workspace.getConfiguration('dxlib').get<string>('visualStudioPath', '') ?? '';
		const same = (a: string, b: string): boolean => path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase();
		const chosen = wanted ? installs.find((i) => same(i.installationPath, wanted)) : undefined;
		const e = { displayName: (chosen ?? installs[0]).displayName, installationVersion: (chosen ?? installs[0]).version };
		const ip = (chosen ?? installs[0]).installationPath;
		const vcvarsall = path.join(ip, 'VC', 'Auxiliary', 'Build', 'vcvarsall.bat');
		return {
			state: fs.existsSync(vcvarsall) ? 'ok' : 'noWorkload',
			displayName: e.displayName,
			version: e.installationVersion,
			installationPath: ip,
			vcvarsall: fs.existsSync(vcvarsall) ? vcvarsall : undefined,
			clPath: findCl(ip),
			msbuild: findMsbuild(ip),
			installs,
			selectionMissing: !!wanted && !chosen,
		};
	}
	const any = await query(['-latest']);
	if (any.length > 0 && any[0].installationPath) {
		const e = any[0];
		return { state: 'noWorkload', displayName: e.displayName, version: e.installationVersion, installationPath: e.installationPath };
	}
	return { state: 'notFound' };
}
