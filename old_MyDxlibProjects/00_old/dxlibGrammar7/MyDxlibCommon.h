#pragma once
#include "Dxlib.h"

namespace WindowSize
{
	constexpr int SIZE_X = 1280;//画面サイズX
	constexpr int SIZE_Y = 720;//画面サイズY
}

namespace ColorCode
{
	//プログラム実行時決まる定数はconst.
	//下の場合だとGetColor関数で計算されてから決まるのでconst.
	const unsigned int BLACK = GetColor(0, 0, 0);//黒
	const unsigned int RED = GetColor(255, 0, 0);//赤
	const unsigned int LIME = GetColor(0, 255, 0);//ライム
	const unsigned int BLUE = GetColor(0, 0, 255);//青
	const unsigned int YELLOW = GetColor(255, 255, 0);//黄
	const unsigned int AQUA = GetColor(0, 255, 255);//アクア
	const unsigned int MAGENTA = GetColor(255, 0, 255);//マゼンタ
}



/*InputKey*/
//KeyInputFlag.キーインプット用のフラグ構造体.
// KeyCodeはintで持つ。
struct IsKeyInput
{
	bool IsNow = false;//押された瞬間True
	bool IsHold = false;//押されている最中True
	int KeyCode;//キーコード
};
/// <summary>
/// 第一引数でキーフラグを渡し、更新を引数で得る.
/// ゲームループ中に毎フレーム実行することで確認している。
/// Enter/Up/Down/Left/Right　の入力を取得する。
/// </summary>
/// <param name="_keyFlag">自身で作成したキーフラグ</param>
/// <returns></returns>
IsKeyInput InputKeyUpdate(IsKeyInput _keyFlag)
{
	//EnterKey.
	if (CheckHitKey(_keyFlag.KeyCode) && !_keyFlag.IsHold)//押されたが、押され続けていない。
	{
		//押された瞬間
		_keyFlag.IsNow = true;
		_keyFlag.IsHold = true;
	}
	else if (CheckHitKey(_keyFlag.KeyCode) && _keyFlag.IsHold)//押されたが、押され続けている.
	{
		//押され続けている
		_keyFlag.IsNow = false;
		_keyFlag.IsHold = true;
	}
	else//押されていない。
	{
		_keyFlag.IsNow = false;
		_keyFlag.IsHold = false;
	}

	return _keyFlag;
}