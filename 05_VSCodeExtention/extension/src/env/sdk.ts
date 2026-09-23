import * as fs from 'fs';
import * as path from 'path';
import * as iconv from 'iconv-lite';

export interface SdkInfo {
	/** 「プロジェクトに追加すべきファイル_VC用」フォルダ。 */
	path: string;
	ok: boolean;
	/** DxLib.h の DXLIB_VERSION_STR_W(例: 3.24f)。 */
	version?: string;
	/** 見つからなかった必須ファイル。 */
	missing: string[];
	/** DxLib_VC 直下の Tool フォルダ。 */
	toolDir?: string;
	/** Tool\ShaderCompiler\ShaderCompiler.exe */
	shaderCompiler?: string;
	/** DxLib_VC\help(リファレンス HTML)。 */
	helpDir?: string;
}

/** VS 2015 以降 x64 で自動リンクされる lib と、必須ヘッダー。 */
const REQUIRED_FILES = [
	'DxLib.h',
	'DxDataType.h',
	'DxDataTypeWin.h',
	'DxFunctionWin.h',
	'DxCompileConfig.h',
	'DxLib_vs2015_x64_MT.lib',
	'DxLib_vs2015_x64_MTd.lib',
	'DxDrawFunc_vs2015_x64_MT.lib',
	'DxDrawFunc_vs2015_x64_MTd.lib',
	'DxUseCLib_vs2015_x64_MT.lib',
	'DxUseCLib_vs2015_x64_MTd.lib',
];

/** DxLib.h は Shift-JIS(CP932)。 */
export function readSdkHeader(sdkPath: string, name: string): string | undefined {
	const file = path.join(sdkPath, name);
	if (!fs.existsSync(file)) {
		return undefined;
	}
	return iconv.decode(fs.readFileSync(file), 'shift_jis');
}

export function inspectSdk(sdkPath: string | undefined): SdkInfo | undefined {
	if (!sdkPath) {
		return undefined;
	}
	const missing = REQUIRED_FILES.filter((f) => !fs.existsSync(path.join(sdkPath, f)));
	let version: string | undefined;
	const header = readSdkHeader(sdkPath, 'DxLib.h');
	if (header) {
		const m = /DXLIB_VERSION_STR_W\s+L"([^"]+)"/.exec(header);
		version = m?.[1];
	}
	const root = path.dirname(sdkPath);
	const toolDir = path.join(root, 'Tool');
	const shaderCompiler = path.join(toolDir, 'ShaderCompiler', 'ShaderCompiler.exe');
	const helpDir = path.join(root, 'help');
	return {
		path: sdkPath,
		ok: missing.length === 0 && version !== undefined,
		version,
		missing,
		toolDir: fs.existsSync(toolDir) ? toolDir : undefined,
		shaderCompiler: fs.existsSync(shaderCompiler) ? shaderCompiler : undefined,
		helpDir: fs.existsSync(helpDir) ? helpDir : undefined,
	};
}
