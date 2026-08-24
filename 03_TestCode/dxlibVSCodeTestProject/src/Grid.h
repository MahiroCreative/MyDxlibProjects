#pragma once

#include "Camera.h"
#include "TileCoord.h"

// マップの盤面を表すクラス。
// 今はマス目の見た目(線・ハイライト・座標表示)だけを持っているが、
// 将来はここに「各マスに何が置かれているか(採掘機・ベルトなど)」の
// データを持たせていく想定の置き場所。
class Grid
{
public:
	Grid();

	// ウィンドウサイズの決定に使うため、Game側に公開しておく。
	int GetScreenWidth() const;
	int GetScreenHeight() const;

	// マウスカーソルが乗っているタイルの座標を求める。範囲外なら x=-1 が返る。
	TileCoord GetHoveredTile(const Camera& camera) const;

	// マス目の線を描画する。
	void DrawGridLines(const Camera& camera) const;

	// マウスが乗っているタイルを半透明でハイライトする。
	void DrawHoverHighlight(const Camera& camera) const;

	// マウスが乗っているタイルの座標をテキストで表示する。
	void DrawTilePositionText(const Camera& camera) const;

private:
	// 1マスの一辺のピクセル数。
	static constexpr int TILE_SIZE = 32;
	// マップの横方向・縦方向のマス数。
	static constexpr int WIDTH = 32;
	static constexpr int HEIGHT = 24;
};
