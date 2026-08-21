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
	static void Update();

	/// <summary>
	/// 何かしらのKeyが押されているかを取得する。
	/// </summary>
	/// <returns></returns>
	static bool isAnyKey();

	/// <summary>
	/// Keyが押された瞬間を取得する。
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="KeyCode">DxlibのKeyCode</param>
	static bool isDownKey(int KeyCode);

	/// <summary>
	/// Keyが押され続けているかを取得する。
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="KeyCode">DxlibのKeyCode</param>
	static bool isHoldKey(int KeyCode);

	/// <summary>
	/// Keyが押されているフレーム数を取得する
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <returns>DxlibのKeyCode</returns>
	static int HoldKeyTime(int KeyCode);

private:
	/*staticメンバ変数*/
	//それぞれのKeyの入力フレーム数
	static int _KeyFrame[];//定義だけ行い実態はcppで生成。
	//どれかのKeyが押されているか
	static bool _isAnyKey;
};