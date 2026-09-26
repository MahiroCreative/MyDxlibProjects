// course の各回のサンプルから、拡張機能で使える .dxtemplate(中身は zip)を作る。
//
// 【置き場所についての注意】拡張機能本体(05_VSCodeExtention/extension/)には**入れない**。
// これらのテンプレートには DxLib SDK 付属のサンプル素材(Tex1.bmp・Kao.bmp・DxChara.x など)が入っており、
// DxLib 作者の権利物で、拡張機能の MIT ライセンスの対象外(CLAUDE.md・06_D3D11Shader/samples/index.html と同じ扱い)。
// 生徒は「新規プロジェクト作成」の「テンプレートファイル (.dxtemplate) を選ぶ...」からこのフォルダのファイルを選ぶ。
//
// 使い方: node course/make_templates.js
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const SAMPLES = path.join(__dirname, '..', 'samples');
const OUT = path.join(__dirname, 'templates');

// ---------------------------------------------------------------------------
// zip の書き方は 05_VSCodeExtention/extension/src/util/zip.ts の writeZip と同じ形式
// (deflate、ファイル名は UTF-8 の汎用フラグ付き)。テンプレートを読む側(templates.ts)と合わせるため、そのまま移した。

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

function crc32(buf) {
	let c = 0xffffffff;
	for (let i = 0; i < buf.length; i++) {
		c = CRC_TABLE[(c ^ buf[i]) & 0xff] ^ (c >>> 8);
	}
	return (c ^ 0xffffffff) >>> 0;
}

function dosDateTime(d) {
	const year = Math.max(1980, d.getFullYear());
	return {
		time: (d.getHours() << 11) | (d.getMinutes() << 5) | Math.floor(d.getSeconds() / 2),
		date: ((year - 1980) << 9) | ((d.getMonth() + 1) << 5) | d.getDate(),
	};
}

function writeZip(entries, now = new Date()) {
	const { time, date } = dosDateTime(now);
	const locals = [];
	const centrals = [];
	let offset = 0;
	for (const e of entries) {
		const name = Buffer.from(e.name.replace(/\\/g, '/'), 'utf8');
		const compressed = zlib.deflateRawSync(e.data);
		const crc = crc32(e.data);
		const local = Buffer.alloc(30);
		local.writeUInt32LE(0x04034b50, 0);
		local.writeUInt16LE(20, 4);
		local.writeUInt16LE(0x0800, 6);
		local.writeUInt16LE(8, 8);
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
		central.writeUInt16LE(20, 4);
		central.writeUInt16LE(20, 6);
		central.writeUInt16LE(0x0800, 8);
		central.writeUInt16LE(8, 10);
		central.writeUInt16LE(time, 12);
		central.writeUInt16LE(date, 14);
		central.writeUInt32LE(crc, 16);
		central.writeUInt32LE(compressed.length, 20);
		central.writeUInt32LE(e.data.length, 24);
		central.writeUInt16LE(name.length, 28);
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

// ---------------------------------------------------------------------------

/** サンプルのフォルダの中身を集める。sample.json・README.md・build 成果物は入れない */
function collectSample(dir) {
	const entries = [];
	const skip = new Set(['sample.json', 'README.md']);
	const walk = (d, rel) => {
		for (const name of fs.readdirSync(d)) {
			if (skip.has(name)) {
				continue;
			}
			const full = path.join(d, name);
			const relName = rel ? `${rel}/${name}` : name;
			if (fs.statSync(full).isDirectory()) {
				if (name === 'bin') {
					continue; // コンパイル済みシェーダーは入れない(生徒側で「すべてコンパイル」する)
				}
				walk(full, relName);
			} else {
				entries.push({ name: relName, data: fs.readFileSync(full) });
			}
		}
	};
	walk(dir, '');
	return entries;
}

const TEMPLATES = [
	{ file: '01_pixel_shader', base: 'C2_PixelShaderTest', name: '講座 01: ピクセルシェーダー', description: '画像の色を変える。シェーダー入門(course/01_pixel_shader.html)。' },
	{ file: '02_constant_buffer', base: 'C5_SetPSConstFTest', name: '講座 02・03: 定数バッファ', description: 'C++ から値を渡す。テクスチャ座標をずらす課題もこれを使う(course/02_constant_buffer.html、03_uv_distortion.html)。' },
	{ file: '04_vertex_shader', base: 'C1_VertexShaderTest', name: '講座 04: 頂点シェーダー', description: '頂点シェーダーで板を動かす(course/04_vertex_shader.html)。' },
	{ file: '05_matrix', base: 'C3_SetVSConstFMtxTest', name: '講座 05: 行列で回す', description: 'C++ の行列を渡して回す(course/05_matrix.html)。' },
	{ file: '06_sprite_vertex', base: 'D1_Sprite2DVertexShader', name: '講座 06: 2D を頂点シェーダーで動かす', description: '正射影カメラ + 3D の板で、2D の絵に頂点シェーダーを使う(course/06_sprite_vertex.html)。' },
	{ file: '07a_model_nolight', base: 'A01_NormalMesh_NoLight', name: '講座 07: モデル(ライト無し)', description: 'MV1 モデルを自作シェーダーで描く基本(course/07_model_lighting.html)。' },
	{ file: '07b_model_dirlight', base: 'A05_NormalMesh_DirLight', name: '講座 07・08: モデル(ディレクショナルライト)', description: '頂点ごとのライト計算。08 でピクセルごとの計算と見比べる(course/07_model_lighting.html、08_pixel_lighting.html)。' },
	{ file: '08b_model_phong', base: 'A11_NormalMesh_DirLight_Phong', name: '講座 08: モデル(ピクセルライト)', description: 'ピクセルごとのライト計算(フォンシェーディング)(course/08_pixel_lighting.html)。' },
	{ file: '09_toon', base: 'A15_SkinMesh4_DirLight_Toon', name: '講座 09: トゥーン', description: 'アニメ調の陰影(course/09_toon.html)。' },
	{ file: '10a_mirror', base: 'B6_Mirror', name: '講座 10: 鏡', description: '描画先に描いて後で使う(鏡)(course/10_render_target.html)。' },
	{ file: '10b_shadow', base: 'B1_3DAction_DepthShadow', name: '講座 10: 影', description: '描画先に描いて後で使う(深度シャドウ)(course/10_render_target.html)。' },
];

fs.mkdirSync(OUT, { recursive: true });
for (const t of TEMPLATES) {
	const src = path.join(SAMPLES, t.base);
	if (!fs.existsSync(src)) {
		throw new Error(`サンプルが無い: ${t.base}`);
	}
	const entries = [{ name: 'template.json', data: Buffer.from(JSON.stringify({ name: t.name, description: t.description }, null, '\t'), 'utf8') }, ...collectSample(src)];
	const out = path.join(OUT, `${t.file}.dxtemplate`);
	fs.writeFileSync(out, writeZip(entries));
	console.log('ok', t.file, `${(fs.statSync(out).size / 1024).toFixed(0)} KB`, `(${entries.length} files)`);
}
