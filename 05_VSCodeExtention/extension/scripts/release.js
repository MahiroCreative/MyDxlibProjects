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

// フォルダごとは消さず、中身だけ入れ替える。install.bat の画面を開いたままだと
// そのフォルダが使用中になり、フォルダの削除が EPERM で失敗するため。
fs.mkdirSync(outDir, { recursive: true });
for (const f of fs.readdirSync(outDir)) {
	fs.rmSync(path.join(outDir, f), { recursive: true, force: true });
}
fs.copyFileSync(vsix, path.join(outDir, vsixName));
// .bat と .txt は改行を CRLF にそろえる(LF だけの bat は goto が壊れることがある)。
// git の取り出し方(autocrlf や ZIP ダウンロード)で LF になっていても直す。
for (const f of fs.readdirSync(path.join(root, 'release-files'))) {
	const src = path.join(root, 'release-files', f);
	const dst = path.join(outDir, f);
	if (/\.(bat|txt)$/i.test(f)) {
		const text = fs.readFileSync(src, 'latin1').replace(/\r?\n/g, '\r\n');
		fs.writeFileSync(dst, text, 'latin1');
	} else {
		fs.copyFileSync(src, dst);
	}
}
console.log(`[release] ${outDir}`);
for (const f of fs.readdirSync(outDir)) {
	console.log(`  ${f} (${fs.statSync(path.join(outDir, f)).size} bytes)`);
}
