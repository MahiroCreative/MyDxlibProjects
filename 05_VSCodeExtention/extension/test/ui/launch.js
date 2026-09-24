// 段階 9 用の VSCode を起動する(検証用プロファイル。フォルダを 1 つ開く、または空の窓)。
// 使い方: node test/ui/launch.js <作業フォルダ> [開くフォルダ]
const path = require('path');
const { spawn } = require('child_process');

const work = path.resolve(process.argv[2]);
const folder = process.argv[3];
const env = { ...process.env };
delete env.ELECTRON_RUN_AS_NODE;
const exe = path.join(process.env.LOCALAPPDATA, 'Programs', 'Microsoft VS Code', 'Code.exe');
const args = ['--user-data-dir', path.join(work, 'user-data'), '--extensions-dir', path.join(work, 'extensions'), '--new-window', '--skip-release-notes', '--disable-workspace-trust'];
if (folder) {
	args.push(folder);
}
const child = spawn(exe, args, { env, detached: true, stdio: 'ignore' });
child.unref();
console.log(`launched pid=${child.pid}`);
