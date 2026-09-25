import * as fs from 'fs';
import * as path from 'path';
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

async function detectVisualStudioActual(): Promise<VsInfo> {
	const withTools = await query(['-latest', '-requires', VC_TOOLS_COMPONENT]);
	if (withTools.length > 0 && withTools[0].installationPath) {
		const e = withTools[0];
		const ip = e.installationPath as string;
		const vcvarsall = path.join(ip, 'VC', 'Auxiliary', 'Build', 'vcvarsall.bat');
		return {
			state: fs.existsSync(vcvarsall) ? 'ok' : 'noWorkload',
			displayName: e.displayName,
			version: e.installationVersion,
			installationPath: ip,
			vcvarsall: fs.existsSync(vcvarsall) ? vcvarsall : undefined,
			clPath: findCl(ip),
		};
	}
	const any = await query(['-latest']);
	if (any.length > 0 && any[0].installationPath) {
		const e = any[0];
		return { state: 'noWorkload', displayName: e.displayName, version: e.installationVersion, installationPath: e.installationPath };
	}
	return { state: 'notFound' };
}
