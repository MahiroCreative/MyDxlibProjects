// 配布用フォルダ(release/)を作る: VSIX + install.bat + README.txt。
// 使い方: npm run release   (vsce package のあとに実行される)
// release/ をそのまま zip にして生徒に配る。
const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..');
const pkg = require(path.join(root, 'package.json'));
const vsixName = `${pkg.name}-${pkg.version}.vsix`;
const vsix = path.join(root, vsixName);
const outDir = path.join(root, 'release');

if (!fs.existsSync(vsix)) {
	console.error(`[release] ${vsixName} がありません。先に vsce package を実行してください。`);
	process.exit(1);
}

fs.rmSync(outDir, { recursive: true, force: true });
fs.mkdirSync(outDir);
fs.copyFileSync(vsix, path.join(outDir, vsixName));
for (const f of fs.readdirSync(path.join(root, 'release-files'))) {
	fs.copyFileSync(path.join(root, 'release-files', f), path.join(outDir, f));
}
console.log(`[release] ${outDir}`);
for (const f of fs.readdirSync(outDir)) {
	console.log(`  ${f} (${fs.statSync(path.join(outDir, f)).size} bytes)`);
}
