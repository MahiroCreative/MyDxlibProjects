#pragma once
namespace MyEngine
{
	/// <summary>
	/// DxLib::DrawGraphと同じ間隔でシェーダを用いて描画
	/// </summary>
	/// <param name="x">X座標</param>
	/// <param name="y">Y座標</param>
	/// <param name="psH">使用するピクセルシェーダ</param>
	/// <param name="firstH">使用画像１</param>
	/// <param name="secondH">使用画像２</param>
	/// <param name="thirdH">使用画像３</param>
	/// <param name="fourthH">使用画像４</param>
	/// <param name="fifthH">使用画像５</param>
	/// <param name="sixthH">使用画像６</param>
	/// <param name="isTrans">true:透過する /false:透過しない</param>
	void DrawGraph(int x, int y, int psH, int firstH, int secondH = -1, int thirdH = -1, int fourthH = -1, int fifthH = -1, int sixthH = -1, bool isTrans = false);
	/// <summary>
	/// DxLib::DrawRotaGraphと同じ間隔でシェーダを用いて描画
	/// </summary>
	/// <param name="x">X座標</param>
	/// <param name="y">Y座標</param>
	/// <param name="psH">使用するピクセルシェーダ</param>
	/// <param name="firstH">使用画像１</param>
	/// <param name="secondH">使用画像２</param>
	/// <param name="thirdH">使用画像３</param>
	/// <param name="fourthH">使用画像４</param>
	/// <param name="fifthH">使用画像５</param>
	/// <param name="sixthH">使用画像６</param>
	/// <param name="isTrans">true:透過する /false:透過しない</param>
	void DrawRotaGraph(int x, int y, float scale, float angle, int psH, int firstH, int secondH = -1, int thirdH = -1, int fourthH = -1, int fifthH = -1, int sixthH = -1, bool isTrans = false);
}


