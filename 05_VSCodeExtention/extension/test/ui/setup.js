// 段階 9(クリック操作による手動確認の代わり)の準備。
// 新しい検証用プロファイルを作り、配布物そのもの(release/ の VSIX)と C/C++ 拡張をインストールする。
// 使い方: node test/ui/setup.js <作業フォルダ> <SDK フォルダ> <cpptools の VSIX>
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');

const work = path.resolve(process.argv[2]);
const sdk = process.argv[3];
const cpptoolsVsix = process.argv[4];
const extRoot = path.resolve(__dirname, '..', '..');
const vsix = fs.readdirSync(path.join(extRoot, 'release')).find((f) => f.endsWith('.vsix'));
if (!vsix) {
	throw new Error('release/ に VSIX がありません。先に npm run release');
}

fs.rmSync(work, { recursive: true, force: true });
for (const d of ['user-data/User', 'extensions', 'projects', 'templates', 'shots']) {
	fs.mkdirSync(path.join(work, d), { recursive: true });
}
fs.writeFileSync(
	path.join(work, 'user-data', 'User', 'settings.json'),
	JSON.stringify(
		{
			'dxlib.sdkPath': sdk,
			'security.workspace.trust.enabled': false,
			'workbench.startupEditor': 'none',
			'extensions.autoUpdate': 'off',
			'update.mode': 'none',
			'telemetry.telemetryLevel': 'off',
			'window.restoreWindows': 'none',
			'workbench.tips.enabled': false,
		},
		null,
		2,
	),
);

const env = { ...process.env };
delete env.ELECTRON_RUN_AS_NODE;
const code = path.join(process.env.LOCALAPPDATA, 'Programs', 'Microsoft VS Code', 'bin', 'code.cmd');
for (const v of [cpptoolsVsix, path.join(extRoot, 'release', vsix)]) {
	const out = execFileSync('cmd.exe', ['/d', '/c', code, '--user-data-dir', path.join(work, 'user-data'), '--extensions-dir', path.join(work, 'extensions'), '--install-extension', v, '--force'], { env, encoding: 'utf8' });
	console.log(out.trim().split(/\r?\n/).pop());
}
console.log(fs.readdirSync(path.join(work, 'extensions')).filter((n) => n !== 'extensions.json').join('\n'));
