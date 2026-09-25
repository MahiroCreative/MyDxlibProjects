import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { CppToolsState, cpptoolsInstalled, intelliSenseState } from './cpptools';
import { inspectSdk, SdkInfo } from './sdk';
import { detectVisualStudio, VsInfo } from './vswhere';
import { findVsProject } from '../build/vsProject';

export interface EnvironmentStatus {
	vs: VsInfo;
	sdkPath: string;
	sdk: SdkInfo | undefined;
	cpptools: CppToolsState;
	/** 開いているフォルダ(最初のワークスペースフォルダ。DxLib プロジェクトとは限らない)。 */
	project?: { name: string; path: string };
	/** DxLib プロジェクトではないが、Visual Studio で作ったプロジェクト(.vcxproj)があるとき、その名前(DESIGN.md 6.1 章)。 */
	vsProject?: string;
	/** 開いているフォルダが、このツールで作った(dxlib タスクを持つ)DxLib プロジェクトかどうか。 */
	isDxLibProject: boolean;
}

export function getConfig<T>(key: string, defaultValue: T, scope?: vscode.ConfigurationScope): T {
	return vscode.workspace.getConfiguration('dxlib', scope).get<T>(key, defaultValue);
}

export async function setGlobalConfig(key: string, value: unknown): Promise<void> {
	await vscode.workspace.getConfiguration('dxlib').update(key, value, vscode.ConfigurationTarget.Global);
}

export function currentFolder(): vscode.WorkspaceFolder | undefined {
	return vscode.workspace.workspaceFolders?.[0];
}

/** 実行ファイル名に使うプロジェクト名。フォルダ名から使えない文字を落とす。 */
export function projectExeName(folder: vscode.WorkspaceFolder): string {
	const base = path.basename(folder.uri.fsPath).replace(/[^A-Za-z0-9_\-]/g, '_');
	return base.length > 0 ? base : 'game';
}

/**
 * このツールが作った DxLib プロジェクトかどうかを、.vscode/tasks.json に
 * dxlib タスクがあるかで判定する。無関係なフォルダを開いただけでビルド/実行
 * ボタンを出さないようにするための判定。
 */
export function isDxLibProject(folder: vscode.WorkspaceFolder): boolean {
	try {
		const tasksFile = path.join(folder.uri.fsPath, '.vscode', 'tasks.json');
		if (!fs.existsSync(tasksFile)) {
			return false;
		}
		const json = JSON.parse(fs.readFileSync(tasksFile, 'utf8')) as { tasks?: Array<{ type?: string }> };
		return Array.isArray(json.tasks) && json.tasks.some((t) => t.type === 'dxlib');
	} catch {
		return false;
	}
}

/**
 * DxLib SDK が使えない理由(使えるなら undefined)。
 * プロジェクトの作成可否の判定に使う。SDK が正しくない間は作成できない。
 */
export function sdkProblem(): string | undefined {
	const sdkPath = getConfig<string>('sdkPath', '');
	if (!sdkPath) {
		return 'DxLib SDK が設定されていません。';
	}
	const sdk = inspectSdk(sdkPath);
	if (!sdk?.version) {
		return 'DxLib SDK のフォルダが正しくありません(DxLib.h が見つかりません)。';
	}
	if (!sdk.ok) {
		return `DxLib SDK のファイルが足りません: ${sdk.missing.join(', ')}。`;
	}
	return undefined;
}

export const SDK_FIX_HINT = 'DxLib パネルの SDK の「変更」から、正しいフォルダを指定してください。';

export async function collectEnvironment(): Promise<EnvironmentStatus> {
	const vs = await detectVisualStudio();
	const sdkPath = getConfig<string>('sdkPath', '');
	const sdk = inspectSdk(sdkPath || undefined);
	const cpptools: CppToolsState = cpptoolsInstalled() ? intelliSenseState.get() : 'missing';
	const folder = currentFolder();
	return {
		vs,
		sdkPath,
		sdk,
		cpptools,
		project: folder ? { name: projectExeName(folder), path: folder.uri.fsPath } : undefined,
		isDxLibProject: folder ? isDxLibProject(folder) : false,
		vsProject: folder && !isDxLibProject(folder) ? findVsProject(folder.uri.fsPath) : undefined,
	};
}
