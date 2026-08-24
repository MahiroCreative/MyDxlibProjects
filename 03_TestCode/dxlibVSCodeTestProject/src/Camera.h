#pragma once

// ワールド座標(マップ上の絶対位置)と画面座標(ウィンドウ上のピクセル位置)の
// 変換だけを担当するクラス。
//
// 今はカメラ位置が常に(0, 0)でスクロール機能はないが、
// 「画面座標に変換する処理」を全部このクラスに閉じ込めておくことで、
// 将来マップスクロールを実装するときはこのクラスの中身だけ変えればよくなる。
class Camera
{
public:
	Camera();

	// ワールド座標 -> 画面座標
	int WorldToScreenX(int worldX) const;
	int WorldToScreenY(int worldY) const;

	// 画面座標 -> ワールド座標(逆変換。マウス位置からタイルを求めるときに使う)
	int ScreenToWorldX(int screenX) const;
	int ScreenToWorldY(int screenY) const;

private:
	// カメラが映しているワールド座標上の原点位置。
	int m_x;
	int m_y;
};
