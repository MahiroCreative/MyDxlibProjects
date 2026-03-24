#include "InputKey.h"

void InputKey::Update()
{
	// 現在のキーの入力状態
	char tmpKey[256];
	// 全てのキーの入力状態を取得
	GetHitKeyStateAll(tmpKey);
	// 何かのキーが押されているかのフラグを初期化
	_isAnyKey = false;
	//全てのKeyの入力状態の確認
	for (int i = 0; i < 256; i++)
	{
		//i番のKeyが押されているかを判定
		if (tmpKey[i] != 0)
		{
			//押しているKeyのフレーム数加算
			_KeyFrame[i]++;
			//何かのKeyが押されていて、かつ押されているKeyのフレーム数が1の時
			if (_KeyFrame[i] == 1) { _isAnyKey = true; }
		}
		else
		{
			//押されていないKeyのフレーム数初期化
			_KeyFrame[i] = 0;
		}
	}
}

bool InputKey::isAnyKey()
{
	return _isAnyKey;
}

bool InputKey::isDownKey(int KeyCode)
{
	//Keyが押されている時間が1フレームの時
	if (_KeyFrame[KeyCode] == 1)
	{
		return true;
	}
	return false;
}

bool InputKey::isHoldKey(int KeyCode)
{
	//Keyが押されている時間が1フレーム以上の時
	if (_KeyFrame[KeyCode] >= 1)
	{
		return true;
	}
	return false;
}

int InputKey::HoldKeyTime(int KeyCode)
{
	//Keyが押されている時間を返す
	if (_KeyFrame[KeyCode] >= 1)
	{
		return _KeyFrame[KeyCode];
	}
	return 0;
}


//静的メンバ変数の実態作成
int InputKey::_KeyFrame[256];
bool InputKey::_isAnyKey = false;