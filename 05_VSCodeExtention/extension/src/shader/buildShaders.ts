import * as fs from 'fs';
import { compileShaderSet, listShaderSources, ShaderJob } from './shaderCore';

/**
 * ビルドの前にシェーダーをコンパイルする(DESIGN.md 9.2 章)。dist/buildShaders.js になる。
 * ビルド用 bat が、VSCode 本体(Code.exe)を ELECTRON_RUN_AS_NODE=1 で Node として動かして呼ぶ。
 * 使い方: Code.exe dist/buildShaders.js <設定の .json>
 * 設定は拡張が bat と同じ場所に書く: ShaderJob の項目 + log(ビルドのログ。エラーの行を足す)。
 * 終了コード: 0 = 成功またはシェーダーが無い、1 = 失敗(ビルドを止める)。
 */

interface BuildShaderConfig extends Omit<ShaderJob, 'only' | 'changedOnly' | 'reportSkipped'> {
	log: string;
}

async function main(): Promise<number> {
	const config = JSON.parse(fs.readFileSync(process.argv[2], 'utf8')) as BuildShaderConfig;
	// シェーダーの無いプロジェクトでは何も出さない(以前のビルドと同じ見た目)
	if (listShaderSources(config.srcDir).length === 0) {
		return 0;
	}
	if (!config.compiler || !fs.existsSync(config.compiler)) {
		console.log('[DxLib] SDK の Tool\\ShaderCompiler\\ShaderCompiler.exe が見つからないので、シェーダーをコンパイルできません。DxLib パネルで SDK フォルダを確認してください。');
		return 1;
	}
	const r = await compileShaderSet({ ...config, changedOnly: true }, (line) => console.log(line));
	if (r.diagnostics.length > 0) {
		// ビルドの赤線(BuildDiagnostics)はこのログを読む。MSBuild は後から同じログに書き足す(-flp の append)
		fs.appendFileSync(config.log, r.diagnostics.join('\r\n') + '\r\n', 'utf8');
	}
	if (r.failed > 0) {
		console.log(`[DxLib] シェーダーのコンパイルに失敗しました (${r.failed} 本)`);
		return 1;
	}
	if (r.compiled === 0) {
		console.log(`[DxLib] シェーダー: 変更なし (${r.upToDate} 本)`);
	}
	return 0;
}

main().then(
	(code) => process.exit(code),
	(e) => {
		console.log(`[DxLib] シェーダーのコンパイル中にエラーが起きました: ${e instanceof Error ? e.message : String(e)}`);
		process.exit(1);
	},
);
