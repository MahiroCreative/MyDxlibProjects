#pragma once
#include "DxLib.h"

/*入出力系*/
/// <summary>
/// staticメンバのみのクラス。Keybord入力を取得する。
/// Update()を毎フレーム行うことでKeyの入力状態を更新する。
/// </summary>
class InputKey
{
public:
	/*staticメンバ関数*/
	/// <summary>
	/// Keyの入力状態の更新。
	/// 毎フレームごとに実行することで更新する。
	/// </summary>
	static inline void Update()
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

	/// <summary>
	/// 何かしらのKeyが押されているかを取得する。
	/// </summary>
	/// <returns></returns>
	static inline bool isAnyKey() { return _isAnyKey; }

	/// <summary>
	/// Keyが押された瞬間を取得する。
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="KeyCode">DxlibのKeyCode</param>
	static inline bool isDownKey(int KeyCode)
	{
		//Keyが押されている時間が1フレームの時
		if (_KeyFrame[KeyCode] == 1)
		{
			return true;
		}
		return false;
	}

	/// <summary>
	/// Keyが押され続けているかを取得する。
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="KeyCode">DxlibのKeyCode</param>
	static bool inline isHoldKey(int KeyCode)
	{
		return _KeyFrame[KeyCode] >= 1;
	}

	/// <summary>
	/// Keyが押されているフレーム数を取得する
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <returns>DxlibのKeyCode</returns>
	static inline int HoldKeyTime(int KeyCode)
	{
		return (_KeyFrame[KeyCode] >= 1) ? _KeyFrame[KeyCode] : 0;
	}

private:
	/*staticメンバ変数*/
	//それぞれのKeyの入力フレーム数
	inline static int _KeyFrame[256] = { 0 };
	//どれかのKeyが押されているか
	inline static bool _isAnyKey = false;
};