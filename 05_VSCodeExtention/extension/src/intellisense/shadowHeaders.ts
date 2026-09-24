import * as crypto from 'crypto';
import * as fs from 'fs';
import * as path from 'path';
import * as iconv from 'iconv-lite';

/**
 * IntelliSense(C/C++ 拡張)に読ませる、DxLib ヘッダーの写し(DESIGN.md 7 章)。
 *
 * C/C++ 拡張は DxLib のヘッダー(CP932)を UTF-8 として読むので、ホバーの説明が文字化けする。
 * さらに DxLib.h は説明を宣言の行末に書く形なので、C/C++ 拡張は 1 行上の関数の説明を拾ってしまう。
 * そこで SDK フォルダの *.h を UTF-8(BOM 付き)に変換し、extern 宣言の行末コメントを
 * `///< 説明`(その行の宣言の説明)に変えた写しを作る。行数は変えないので、行番号は元と一致する。
 * ビルドは元の SDK を使う。
 */

/** 変換の方式を変えたら上げる。識別子が変わり、古い写しが作り直される。 */
const SHADOW_FORMAT = 2;

/** 写しのフォルダに置く、元の SDK のパスを書いたファイル。写しが完成している目印も兼ねる。 */
export const SHADOW_SOURCE_FILE = 'source.txt';

/**
 * extern 宣言の行末コメント `// 説明` を、その宣言自身の説明を表す `///< 説明` に変える。それ以外の行はそのまま。
 * C/C++ 拡張は `///<` を同じ行の宣言の説明として扱い、1 行上の別の関数の説明を拾わなくなる。
 * (2026-09-24 に置き方を実験: 宣言の前の `/** *\/` や 1 行上の行末は拾わず、`///<` と 1 行上の `/** *\/` は拾った。
 * 行数を変えずに済むのは `///<`)
 */
export function convertTrailingComments(text: string): string {
	return text
		.split('\n')
		.map((line) => {
			const cr = line.endsWith('\r') ? '\r' : '';
			const body = cr ? line.slice(0, -1) : line;
			const m = /^(\s*extern\s[^/]*?;)\s*\/\/(?![/!])\s*(.*?)\s*$/.exec(body);
			if (!m || !m[2]) {
				return line;
			}
			return `${m[1]} ///< ${m[2]}${cr}`;
		})
		.join('\n');
}

/**
 * 写しを用意してそのフォルダを返す。すでに同じ SDK(パスと DxLib.h の更新日時・大きさが同じ)の写しがあれば作らない。
 * 作れなければ undefined(呼び出し側は元の SDK をそのまま使う)。
 */
export function ensureShadowHeaders(storageDir: string, sdkPath: string): string | undefined {
	let stat: fs.Stats;
	try {
		stat = fs.statSync(path.join(sdkPath, 'DxLib.h'));
	} catch {
		return undefined;
	}
	const id = crypto.createHash('md5').update(`${SHADOW_FORMAT}|${sdkPath.toLowerCase()}|${stat.mtimeMs}|${stat.size}`).digest('hex').slice(0, 12);
	const root = path.join(storageDir, 'intellisense');
	const dir = path.join(root, id);
	if (fs.existsSync(path.join(dir, SHADOW_SOURCE_FILE))) {
		return dir;
	}
	try {
		// 途中で失敗しても半端な写しを使わないよう、一時フォルダに作ってから名前を変える
		const tmp = `${dir}.tmp`;
		fs.rmSync(tmp, { recursive: true, force: true });
		fs.mkdirSync(tmp, { recursive: true });
		for (const name of fs.readdirSync(sdkPath)) {
			if (!/\.h$/i.test(name)) {
				continue;
			}
			const text = iconv.decode(fs.readFileSync(path.join(sdkPath, name)), 'cp932');
			fs.writeFileSync(path.join(tmp, name), '﻿' + convertTrailingComments(text), 'utf8');
		}
		fs.writeFileSync(path.join(tmp, SHADOW_SOURCE_FILE), sdkPath, 'utf8');
		fs.rmSync(dir, { recursive: true, force: true });
		fs.renameSync(tmp, dir);
		// 古い写し(別の SDK・別の版)を片づける
		for (const other of fs.readdirSync(root)) {
			if (other !== id) {
				fs.rmSync(path.join(root, other), { recursive: true, force: true });
			}
		}
		return dir;
	} catch {
		return undefined;
	}
}
