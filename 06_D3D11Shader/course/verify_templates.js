// course/templates/*.dxtemplate が、拡張機能の zip の読み方(extension/src/util/zip.ts の readZip)で
// 正しく読めるかを確かめる(読み方をそのまま移したもの。iconv-lite が要らない道筋だけ使う)。
// 使い方: node course/verify_templates.js
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

function readZip(buf) {
	let end = -1;
	for (let i = buf.length - 22; i >= Math.max(0, buf.length - 22 - 0xffff); i--) {
		if (buf.readUInt32LE(i) === 0x06054b50) {
			end = i;
			break;
		}
	}
	if (end < 0) {
		throw new Error('zip ファイルではありません。');
	}
	const count = buf.readUInt16LE(end + 10);
	let p = buf.readUInt32LE(end + 16);
	const result = [];
	for (let i = 0; i < count; i++) {
		if (buf.readUInt32LE(p) !== 0x02014b50) {
			throw new Error('zip ファイルが壊れています。');
		}
		const flags = buf.readUInt16LE(p + 8);
		const method = buf.readUInt16LE(p + 10);
		const compSize = buf.readUInt32LE(p + 20);
		const size = buf.readUInt32LE(p + 24);
		const nameLen = buf.readUInt16LE(p + 28);
		const extraLen = buf.readUInt16LE(p + 30);
		const commentLen = buf.readUInt16LE(p + 32);
		const localOffset = buf.readUInt32LE(p + 42);
		const rawName = buf.subarray(p + 46, p + 46 + nameLen);
		if (!(flags & 0x0800)) {
			throw new Error('UTF-8 フラグが無い(想定外)');
		}
		const name = rawName.toString('utf8').replace(/\\/g, '/');
		p += 46 + nameLen + extraLen + commentLen;
		if (name.endsWith('/')) {
			continue;
		}
		if (buf.readUInt32LE(localOffset) !== 0x04034b50) {
			throw new Error('zip ファイルが壊れています(ローカルヘッダ)。');
		}
		const lNameLen = buf.readUInt16LE(localOffset + 26);
		const lExtraLen = buf.readUInt16LE(localOffset + 28);
		const start = localOffset + 30 + lNameLen + lExtraLen;
		const raw = buf.subarray(start, start + compSize);
		const data = method === 8 ? zlib.inflateRawSync(raw) : Buffer.from(raw);
		if (data.length !== size) {
			throw new Error(`長さが合わない: ${name}`);
		}
		result.push({ name, data });
	}
	return result;
}

const DIR = path.join(__dirname, 'templates');
let ng = 0;
for (const f of fs.readdirSync(DIR).filter((f) => f.endsWith('.dxtemplate'))) {
	try {
		const entries = readZip(fs.readFileSync(path.join(DIR, f)));
		const tpl = entries.find((e) => e.name === 'template.json');
		if (!tpl) {
			throw new Error('template.json が無い');
		}
		const json = JSON.parse(tpl.data.toString('utf8').replace(/^﻿/, ''));
		if (!json.name) {
			throw new Error('name が無い');
		}
		const hasMain = entries.some((e) => e.name === 'src/main.cpp');
		if (!hasMain) {
			throw new Error('src/main.cpp が無い');
		}
		console.log(`OK ${f}  name=${json.name}  files=${entries.length}`);
	} catch (e) {
		ng++;
		console.log(`NG ${f}  ${e.message}`);
	}
}
console.log(`\nNG ${ng}`);
process.exit(ng ? 1 : 0);
