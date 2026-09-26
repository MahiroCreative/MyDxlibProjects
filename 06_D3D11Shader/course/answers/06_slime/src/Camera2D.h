// 2D の絵を「正射影カメラ + 3D の板」で描くための補助。
//
// DxLib の 2D の描画(DrawPrimitive2DToShader など)は、自作の頂点シェーダーを使えない(DxLib の内蔵の頂点シェーダーが必ず使われる)。
// 3D の描画(DrawPolygon3DToShader)なら自作の頂点シェーダーが使えるので、カメラを
//   - 正射影(遠くても小さくならない)
//   - 3D の座標 1 が画面の 1 画素、x は右・y は下(DrawGraph と同じ向き)
// に設定して、2D と同じ見た目のまま 3D の関数で描く。Unity の 2D と同じ考え方。
#pragma once
#include "DxLib.h"
#include <vector>

// 描画先の画素の座標と 3D の座標がそろうようにカメラを設定する。
// SetDrawScreen はカメラの設定を元に戻すので、SetDrawScreen の後に毎回呼ぶ。
inline void SetupCamera2D()
{
	int w, h;
	GetDrawScreenSize(&w, &h);

	// 正射影。引数は画面の縦方向に映る範囲(3D の座標の大きさ)。画面の高さと同じにすると 1 画素 = 1
	SetupCamera_Ortho((float)h);

	// 板は z = 0 に置く。カメラは z = 500 から、z がマイナスの向きを見る
	SetCameraNearFar(1.0f, 1000.0f);

	// カメラの上方向を y のマイナスにすると、画面の上ほど y が小さくなる(DrawGraph と同じ)。
	// +z 側から見ているので、x はそのまま右が大きくなる(-z 側から見ると左右が反対になる)
	SetCameraPositionAndTargetAndUpVec(
		VGet(w / 2.0f, h / 2.0f, 500.0f),	// カメラの位置: 画面の中心の真上
		VGet(w / 2.0f, h / 2.0f, 0.0f),	// 見る所: 画面の中心
		VGet(0.0f, -1.0f, 0.0f));			// 上方向
}

// 画面の四角い範囲(左上 x, y、幅 w、高さ h)に画像を貼る板を、縦横 divX × divY のマス目に分けて作る。
// 頂点シェーダーで頂点を動かして形を変えるには、マス目に分けて頂点を増やしておく(4 隅だけでは曲げられない)。
// 返す頂点は三角形のリスト(DrawPolygon3DToShader にそのまま渡す。三角形の数は divX * divY * 2)。
inline std::vector<VERTEX3DSHADER> MakeSpriteGrid(float x, float y, float w, float h, int divX, int divY, COLOR_U8 color)
{
	std::vector<VERTEX3DSHADER> v;
	v.reserve((size_t)divX * divY * 6);

	auto point = [&](int ix, int iy)
	{
		VERTEX3DSHADER p = {};	// 使わない要素(spos, tan, binorm など)も 0 にしておく
		const float u = (float)ix / divX;
		const float t = (float)iy / divY;
		p.pos = VGet(x + w * u, y + h * t, 0.0f);
		p.norm = VGet(0.0f, 0.0f, 1.0f);
		p.dif = color;
		p.spc = GetColorU8(0, 0, 0, 0);
		p.u = u;
		p.v = t;
		return p;
	};

	for (int iy = 0; iy < divY; iy++)
	{
		for (int ix = 0; ix < divX; ix++)
		{
			// マス 1 個を三角形 2 個で
			v.push_back(point(ix, iy));
			v.push_back(point(ix + 1, iy));
			v.push_back(point(ix, iy + 1));
			v.push_back(point(ix + 1, iy));
			v.push_back(point(ix + 1, iy + 1));
			v.push_back(point(ix, iy + 1));
		}
	}
	return v;
}
