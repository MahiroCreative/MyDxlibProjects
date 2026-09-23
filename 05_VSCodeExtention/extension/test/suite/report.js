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

module.exports = { Report, sleep, waitFor };
