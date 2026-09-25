import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';
import { classifyEncoding, convertToUtf8Bom, findVsProject, pickPlatform, projectSources, queryVsProject } from '../build/vsProject';
import { currentFolder, projectExeName } from '../env/environment';
import { detectVisualStudio } from '../env/vswhere';
import { writeProjectFiles } from './createProject';

/**
 * Visual Studio で作ったプロジェクトを、この拡張で使えるようにする(DESIGN.md 6.1 章)。
 * .vscode の一式を書くだけで、.vcxproj・.sln・.slnx は触らない。
 */

export interface AdoptArgs {
	/** BOM なし UTF-8・Shift-JIS のソースを BOM 付き UTF-8 にそろえるか。省略すると確認画面で聞く(検証では渡す)。 */
	convert?: boolean;
}

export interface AdoptResult {
	ok: boolean;
	error?: string;
	vcxproj?: string;
	converted?: string[];
}

export async function adoptVsProject(args: AdoptArgs = {}): Promise<AdoptResult> {
	const folder = currentFolder();
	if (!folder) {
		return { ok: false, error: 'プロジェクトのフォルダが開かれていません。' };
	}
	const dir = folder.uri.fsPath;
	const name = findVsProject(dir);
	if (!name) {
		return { ok: false, error: 'このフォルダに Visual Studio のプロジェクト(.vcxproj)が見つかりません。' };
	}
	const vs = await detectVisualStudio();
	if (vs.state !== 'ok' || !vs.msbuild) {
		return { ok: false, error: 'Visual Studio の C++ ワークロード(MSBuild)が見つかりません。DxLib パネルの環境欄を確認してください。' };
	}
	const vcxproj = path.join(dir, name);
	const platform = pickPlatform(fs.readFileSync(vcxproj, 'utf8'));

	// 1. 文字コード: BOM なし UTF-8 と Shift-JIS は、MSVC が正しく読めない(日本語の文字列が化ける・警告 C4819)
	const sources = projectSources(dir);
	const kinds = sources.map((f) => classifyEncoding(fs.readFileSync(f)));
	const utf8 = kinds.filter((k) => k === 'utf8').length;
	const sjis = kinds.filter((k) => k === 'sjis').length;
	let convert = args.convert;
	if (convert === undefined && utf8 + sjis > 0) {
		const yes = 'そろえる';
		const detail = [utf8 ? `BOM なし UTF-8: ${utf8} 個` : '', sjis ? `Shift-JIS: ${sjis} 個` : ''].filter(Boolean).join('、');
		const pick = await vscode.window.showWarningMessage(
			`ソースの文字コードを BOM 付き UTF-8 にそろえますか?(${detail})`,
			{ modal: true, detail: 'Visual Studio でもそのまま開けます。日本語の文字列の文字化けと、警告 C4819 が直ります。' },
			yes,
			'そのまま',
		);
		if (pick === undefined) {
			return { ok: false, error: 'キャンセルしました。' };
		}
		convert = pick === yes;
	}
	const converted = convert ? convertToUtf8Bom(sources) : [];

	// 2. 出力先と作業フォルダは MSBuild に聞く(Visual Studio と同じ場所)
	const debug = await queryVsProject(vs.msbuild, vcxproj, 'Debug', platform);
	const release = await queryVsProject(vs.msbuild, vcxproj, 'Release', platform);
	if (!debug || !release || !debug.targetPath || !release.targetPath) {
		return { ok: false, error: `${name} の設定を MSBuild から読めませんでした。Visual Studio で開いてビルドできるか確かめてください。` };
	}
	writeProjectFiles(dir, projectExeName(folder), {
		vcxproj: name,
		platform,
		debug: { exe: debug.targetPath, cwd: debug.workingDir },
		release: { exe: release.targetPath, cwd: release.workingDir },
	});
	// そのままにしたとき、Shift-JIS のファイルを VSCode が文字化けせずに開けるよう、文字コードの推測を有効にする
	if (!convert && sjis > 0) {
		const settingsFile = path.join(dir, '.vscode', 'settings.json');
		const settings = JSON.parse(fs.readFileSync(settingsFile, 'utf8').replace(/^﻿/, ''));
		settings['files.autoGuessEncoding'] = true;
		fs.writeFileSync(settingsFile, JSON.stringify(settings, null, '\t') + '\n', 'utf8');
	}
	return { ok: true, vcxproj, converted };
}
