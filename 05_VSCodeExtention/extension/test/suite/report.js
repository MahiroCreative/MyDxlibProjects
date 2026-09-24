// 検証結果を集めて report_<段階>.json に書く小さな道具。1 項目でも NG なら最後に例外を投げる。
const fs = require('fs');
const path = require('path');

class Report {
	constructor(phase) {
		this.phase = phase;
		this.items = [];
	}

	check(name, ok, detail) {
		const item = { name, ok: !!ok, detail: detail === undefined ? '' : String(detail) };
		this.items.push(item);
		console.log(`[${this.phase}] ${item.ok ? 'OK' : 'NG'}  ${name}${item.detail ? '  : ' + item.detail : ''}`);
		return item.ok;
	}

	async step(name, fn) {
		try {
			const r = await fn();
			if (r === undefined || r === true) {
				return this.check(name, true);
			}
			if (r === false) {
				return this.check(name, false);
			}
			return this.check(name, r.ok, r.detail);
		} catch (e) {
			return this.check(name, false, e && e.stack ? e.stack.split('\n').slice(0, 3).join(' | ') : e);
		}
	}

	finish() {
		const file = path.join(process.env.DXLIB_TEST_WORK, `report_${this.phase}.json`);
		fs.writeFileSync(file, JSON.stringify(this.items, null, 2));
		const ng = this.items.filter((i) => !i.ok);
		if (ng.length > 0) {
			throw new Error(`${this.phase}: NG ${ng.length} 件 (${ng.map((i) => i.name).join(', ')})`);
		}
	}
}

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

async function waitFor(fn, timeoutMs, intervalMs = 500) {
	const end = Date.now() + timeoutMs;
	while (Date.now() < end) {
		const v = await fn();
		if (v) {
			return v;
		}
		await sleep(intervalMs);
	}
	return undefined;
}

/**
 * 「新規プロジェクト作成」のボタン送信と同じ経路(dxlib.createProject に引数)で作り、
 * 作成後に頼んだ「フォルダを開く」を横取りして記録する。実際に開くと検証中の窓が置き換わるため。
 */
async function createProjectCapturingOpen(vscode, args) {
	const opened = [];
	const orig = vscode.commands.executeCommand;
	vscode.commands.executeCommand = async (id, ...rest) => {
		if (id === 'vscode.openFolder') {
			opened.push({ uri: rest[0], options: rest[1] });
			return undefined;
		}
		return orig.call(vscode.commands, id, ...rest);
	};
	try {
		await orig.call(vscode.commands, 'dxlib.createProject', args);
	} finally {
		vscode.commands.executeCommand = orig;
	}
	return opened;
}

/**
 * 補完の設定のインクルードパスが、指定した SDK から作ったヘッダーの写しを指しているか(DESIGN.md 7 章)。
 * 写しのフォルダには元の SDK のパスを書いた source.txt がある。
 */
function includesShadowOf(includePath, sdk) {
	const fs = require('fs');
	const path = require('path');
	const same = (a, b) => !!a && !!b && path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase();
	return includePath.some((p) => {
		try {
			return same(fs.readFileSync(path.join(p, 'source.txt'), 'utf8').trim(), sdk) && fs.existsSync(path.join(p, 'DxLib.h'));
		} catch {
			return false;
		}
	});
}

module.exports = { Report, sleep, waitFor, createProjectCapturingOpen, includesShadowOf };
