#include "Grid.h"
#include "DxLib.h"

Grid::Grid()
{
}

int Grid::GetScreenWidth() const
{
	return TILE_SIZE * WIDTH;
}

int Grid::GetScreenHeight() const
{
	return TILE_SIZE * HEIGHT;
}

TileCoord Grid::GetHoveredTile(const Camera& camera) const
{
	// GetMousePointはウィンドウ内でのマウス座標(ピクセル単位)を取得するDxLib関数。
	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	// 画面座標をワールド座標に変換する(変換ロジック自体はCameraクラスに任せる)。
	int worldX = camera.ScreenToWorldX(mouseX);
	int worldY = camera.ScreenToWorldY(mouseY);

	// ピクセル座標をTILE_SIZEで割ることで、何マス目にいるか(タイル座標)が求まる。
	int tileX = worldX / TILE_SIZE;
	int tileY = worldY / TILE_SIZE;

	// マップの範囲内かどうかを判定する。
	// worldX/worldYが負の場合、C++の整数除算は0方向に丸められるため
	// tileXだけでは範囲外判定ができない(例: -1/32は0になってしまう)。
	// そのため割り算前の値でも範囲チェックをしている。
	if (worldX >= 0 && tileX < WIDTH && worldY >= 0 && tileY < HEIGHT)
	{
		return TileCoord{ tileX, tileY };
	}

	// 範囲外のときは「選択なし」を表すデフォルト値(x=-1)を返す。
	return TileCoord{};
}

void Grid::DrawGridLines(const Camera& camera) const
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	// 縦線: x=0からWIDTHまで、TILE_SIZEおきに1本ずつ引く(<=なので右端の線も引かれる)。
	for (int x = 0; x <= WIDTH; ++x)
	{
		int px = camera.WorldToScreenX(x * TILE_SIZE);
		DrawLine(px, 0, px, screenHeight, GetColor(60, 60, 60));
	}
	// 横線も同様。
	for (int y = 0; y <= HEIGHT; ++y)
	{
		int py = camera.WorldToScreenY(y * TILE_SIZE);
		DrawLine(0, py, screenWidth, py, GetColor(60, 60, 60));
	}
}

void Grid::DrawHoverHighlight(const Camera& camera) const
{
	TileCoord tile = GetHoveredTile(camera);

	// 範囲外でカーソルがどのマスにも乗っていない場合は何も描かない。
	if (tile.x < 0) return;

	int px = camera.WorldToScreenX(tile.x * TILE_SIZE);
	int py = camera.WorldToScreenY(tile.y * TILE_SIZE);

	// SetDrawBlendModeで半透明合成を有効にする。
	// DX_BLENDMODE_ALPHAは通常のアルファブレンド、第2引数100は不透明度(0〜255)。
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
	// 最後の引数TRUEで塗りつぶし四角形を描画する(FALSEだと枠線のみになる)。
	DrawBox(px, py, px + TILE_SIZE, py + TILE_SIZE, GetColor(255, 255, 0), TRUE);
	// 他の描画に影響を与えないよう、必ず通常モードに戻しておく。
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Grid::DrawTilePositionText(const Camera& camera) const
{
	TileCoord tile = GetHoveredTile(camera);

	if (tile.x >= 0)
	{
		// DrawFormatStringはprintf風の書式("%d"など)を使える文字列描画関数。
		DrawFormatString(10, 10, GetColor(255, 255, 255), "Tile: (%d, %d)", tile.x, tile.y);
	}
	else
	{
		DrawString(10, 10, "Tile: -", GetColor(255, 255, 255));
	}
}
