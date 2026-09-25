import * as iconv from 'iconv-lite';
import * as zlib from 'zlib';

/**
 * zip の読み書き(テンプレートの zip。DESIGN.md 8 章)。新しい依存を入れないため、形式を自前で扱う。
 * 書くのは deflate(方式 8)、読むのは格納(0)と deflate(8)。書くファイル名は UTF-8(汎用フラグ 11 番)。
 * 読むとき、フラグの無い名前は CP932 として読む(Windows の「圧縮 (zip 形式) フォルダー」で作った zip)。
 * 暗号化・ZIP64・複数ディスクには対応しない(読むときは例外にする)。
 */

export interface ZipEntry {
	/** 区切りは「/」。フォルダの項目は含めない。 */
	name: string;
	data: Buffer;
}

const CRC_TABLE = (() => {
	const t = new Uint32Array(256);
	for (let n = 0; n < 256; n++) {
		let c = n;
		for (let k = 0; k < 8; k++) {
			c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
		}
		t[n] = c >>> 0;
	}
	return t;
})();

function crc32(buf: Buffer): number {
	let c = 0xffffffff;
	for (let i = 0; i < buf.length; i++) {
		c = CRC_TABLE[(c ^ buf[i]) & 0xff] ^ (c >>> 8);
	}
	return (c ^ 0xffffffff) >>> 0;
}

/** DOS 形式の日時(ローカル時刻)。 */
function dosDateTime(d: Date): { time: number; date: number } {
	const year = Math.max(1980, d.getFullYear());
	return {
		time: (d.getHours() << 11) | (d.getMinutes() << 5) | Math.floor(d.getSeconds() / 2),
		date: ((year - 1980) << 9) | ((d.getMonth() + 1) << 5) | d.getDate(),
	};
}

export function writeZip(entries: ZipEntry[], now: Date = new Date()): Buffer {
	const { time, date } = dosDateTime(now);
	const locals: Buffer[] = [];
	const centrals: Buffer[] = [];
	let offset = 0;
	for (const e of entries) {
		const name = Buffer.from(e.name.replace(/\\/g, '/'), 'utf8');
		const compressed = zlib.deflateRawSync(e.data);
		const crc = crc32(e.data);
		const local = Buffer.alloc(30);
		local.writeUInt32LE(0x04034b50, 0);
		local.writeUInt16LE(20, 4); // 展開に必要なバージョン 2.0
		local.writeUInt16LE(0x0800, 6); // ファイル名は UTF-8
		local.writeUInt16LE(8, 8); // deflate
		local.writeUInt16LE(time, 10);
		local.writeUInt16LE(date, 12);
		local.writeUInt32LE(crc, 14);
		local.writeUInt32LE(compressed.length, 18);
		local.writeUInt32LE(e.data.length, 22);
		local.writeUInt16LE(name.length, 26);
		local.writeUInt16LE(0, 28);
		locals.push(local, name, compressed);

		const central = Buffer.alloc(46);
		central.writeUInt32LE(0x02014b50, 0);
		central.writeUInt16LE(20, 4); // 作成したバージョン
		central.writeUInt16LE(20, 6);
		central.writeUInt16LE(0x0800, 8);
		central.writeUInt16LE(8, 10);
		central.writeUInt16LE(time, 12);
		central.writeUInt16LE(date, 14);
		central.writeUInt32LE(crc, 16);
		central.writeUInt32LE(compressed.length, 20);
		central.writeUInt32LE(e.data.length, 24);
		central.writeUInt16LE(name.length, 28);
		// 拡張・コメントの長さ、ディスク番号、内部属性、外部属性は 0
		central.writeUInt32LE(offset, 42);
		centrals.push(central, name);

		offset += local.length + name.length + compressed.length;
	}
	const centralBuf = Buffer.concat(centrals);
	const end = Buffer.alloc(22);
	end.writeUInt32LE(0x06054b50, 0);
	end.writeUInt16LE(entries.length, 8);
	end.writeUInt16LE(entries.length, 10);
	end.writeUInt32LE(centralBuf.length, 12);
	end.writeUInt32LE(offset, 16);
	return Buffer.concat([...locals, centralBuf, end]);
}

export function readZip(buf: Buffer): ZipEntry[] {
	// 終端レコード(末尾のコメントがあっても探せるよう、後ろから探す)
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
	if (count === 0xffff || p === 0xffffffff) {
		throw new Error('この zip の形式(ZIP64)には対応していません。');
	}
	const result: ZipEntry[] = [];
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
		// UTF-8 のフラグが無い名前は、Windows の「圧縮 (zip 形式) フォルダー」では CP932
		const name = (flags & 0x0800 ? rawName.toString('utf8') : iconv.decode(rawName, 'cp932')).replace(/\\/g, '/');
		p += 46 + nameLen + extraLen + commentLen;
		if (flags & 0x0001) {
			throw new Error('暗号化された zip には対応していません。');
		}
		if (name.endsWith('/')) {
			continue; // フォルダの項目
		}
		if (buf.readUInt32LE(localOffset) !== 0x04034b50) {
			throw new Error('zip ファイルが壊れています。');
		}
		const lNameLen = buf.readUInt16LE(localOffset + 26);
		const lExtraLen = buf.readUInt16LE(localOffset + 28);
		const start = localOffset + 30 + lNameLen + lExtraLen;
		const raw = buf.subarray(start, start + compSize);
		let data: Buffer;
		if (method === 0) {
			data = Buffer.from(raw);
		} else if (method === 8) {
			data = zlib.inflateRawSync(raw);
		} else {
			throw new Error(`この zip の圧縮方式(${method})には対応していません。`);
		}
		if (data.length !== size) {
			throw new Error('zip ファイルが壊れています。');
		}
		result.push({ name, data });
	}
	return result;
}
