#pragma once
#include "DxLib.h"


/// <summary>
/// staticメンバのみのクラス。Mouse入力を取得する。
/// Update()を毎フレーム行うことでMouseの入力状態を更新する。
/// </summary>
class InputMouse
{
public:
	/*staticメンバ関数*/
	/// <summary>
	/// Mouseの入力状態の更新。
	/// 毎フレームごとに実行することで更新する。
	/// </summary>
	static void Update();
	/// <summary>
	/// Mouseが押された瞬間を取得する。	
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="MouseCode">DxlibのMouseCode</param>
	static bool isDownMouse(int MouseCode);
	/// <summary>
	/// Mouseが押され続けているかを取得する。
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="MouseCode">DxlibのMouseCode</param>
	static bool isHoldMouse(int MouseCode);
	/// <summary>
	/// Mouseが押されているフレーム数を取得する.
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="MouseCode">DxlibのMouseCode</param>
	static int HoldMouseTime(int MouseCode);
private:
	/*staticメンバ変数*/
	//それぞれのMouseのFrame数
	static int MouseFrame[];//定義だけ行い実態はcppで生成。
};