// 段階 4: シェーダー雛形を DxLib 上で実際に描画して確かめる。
// resources/shaders の雛形を拡張機能と同じ手順(CP932 に変換 → SDK の ShaderCompiler)でコンパイルし、
// main.cpp を拡張機能と同じ cl のオプションでビルドして実行、画素の色で判定する。
// 使い方: node test/shader-runtime/run.js <作業フォルダ> <SDK フォルダ>
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');
const iconv = require('iconv-lite');

function findVcvarsall() {
	const vswhere = path.join(process.env['ProgramFiles(x86)'], 'Microsoft Visual Studio', 'Installer', 'vswhere.exe');
	const out = execFileSync(vswhere, ['-latest', '-products', '*', '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath', '-utf8'], { encoding: 'utf8' }).trim();
	return path.join(out, 'VC', 'Auxiliary', 'Build', 'vcvarsall.bat');
}

function main() {
	const work = path.resolve(process.argv[2], 'shader-runtime');
	const sdk = process.argv[3];
	const templates = path.resolve(__dirname, '..', '..', 'resources', 'shaders');
	const compiler = path.join(path.dirname(sdk), 'Tool', 'ShaderCompiler', 'ShaderCompiler.exe');
	fs.rmSync(work, { recursive: true, force: true });
	fs.mkdirSync(path.join(work, 'src'), { recursive: true });
	fs.mkdirSync(path.join(work, 'shaders', 'src'), { recursive: true });
	fs.mkdirSync(path.join(work, 'shaders', 'bin'), { recursive: true });

	// 1. シェーダー: 雛形そのまま 3 本 + 自作シェーダーが動いている証明用の色反転版 1 本
	const read = (f) => fs.readFileSync(path.join(templates, f), 'utf8').replace(/^\uFEFF/, '');
	const sources = {
		Template2DPS: { text: read('PixelShader2D.hlsl'), target: 'ps_4_0', ext: '.pso' },
		Template3DPS: { text: read('PixelShader3D.hlsl'), target: 'ps_4_0', ext: '.pso' },
		TemplateVS: { text: read('VertexShader.hlsl'), target: 'vs_4_0', ext: '.vso' },
	};
	const invert = sources.Template2DPS.text.replace('return color * input.Diffuse;', 'return float4(1.0f - color.rgb, color.a) * input.Diffuse;');
	if (invert === sources.Template2DPS.text) {
		throw new Error('色反転版を作れない(雛形の return 行が変わった?)');
	}
	sources.Invert2DPS = { text: invert, target: 'ps_4_0', ext: '.pso' };
	for (const [name, s] of Object.entries(sources)) {
		const src = path.join(work, 'shaders', 'src', `${name}.hlsl`);
		fs.writeFileSync(src, iconv.encode(s.text, 'shift_jis'));
		execFileSync(compiler, [`/T${s.target}`, `/Fo${path.join(work, 'shaders', 'bin', name + s.ext)}`, src], { stdio: 'ignore' });
		console.log(`[shader-runtime] compiled ${name}${s.ext}`);
	}

	// 2. C++: 拡張機能のビルドと同じオプション
	fs.copyFileSync(path.join(__dirname, 'main.cpp'), path.join(work, 'src', 'main.cpp'));
	const bat = path.join(work, 'build.bat');
	fs.writeFileSync(
		bat,
		[
			'@echo off',
			'chcp 65001 >nul',
			`call "${findVcvarsall()}" x64 >nul 2>&1`,
			`cd /d "${work}"`,
			'if not exist build mkdir build',
			`cl /nologo /EHsc /W3 /wd4819 /std:c++20 /source-charset:.932 /execution-charset:.932 /D_WINDOWS /DWIN32 /O2 /MT /DNDEBUG /I "${sdk}" src\\main.cpp /Fobuild\\ /Febuild\\shadertest.exe /link /SUBSYSTEM:WINDOWS /LIBPATH:"${sdk}"`,
		].join('\r\n') + '\r\n',
		'utf8',
	);
	execFileSync('cmd.exe', ['/d', '/c', bat], { stdio: 'ignore' });
	const exe = path.join(work, 'build', 'shadertest.exe');
	if (!fs.existsSync(exe)) {
		throw new Error('検証プログラムのビルドに失敗');
	}

	// 3. 実行して判定
	execFileSync(exe, [], { cwd: work, timeout: 60000 });
	const results = fs.readFileSync(path.join(work, 'results.txt'), 'utf8');
	// 判定行は「OK|NG 名前 (x,y) ...」の形。集計行「NG count=0」は数えない。
	const checkLine = /^(OK|NG) \S+ \(\d+,\d+\)/;
	const lines = results.split(/\r?\n/).filter((l) => checkLine.test(l));
	for (const line of lines) {
		console.log(`[phase4] ${line}`);
	}
	const ng = lines.filter((l) => l.startsWith('NG ')).length;
	if (lines.length === 0) {
		throw new Error('判定行が 1 つも無い');
	}
	console.log(`[phase4] 画像: ${path.join(work, 'result.png')}`);
	if (ng > 0) {
		throw new Error(`NG ${ng} 件`);
	}
}

try {
	main();
} catch (e) {
	console.error('[phase4] 失敗:', e.message);
	process.exit(1);
}
