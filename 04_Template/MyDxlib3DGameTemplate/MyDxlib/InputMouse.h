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
	static inline void Update()
	{
		//Mouse入力を取得
		int tempMouse = DxLib::GetMouseInput();

		// 左クリック (MOUSE_INPUT_LEFT = 1)
		if (tempMouse & MOUSE_INPUT_LEFT) { _MouseFrame[0]++; }
		else { _MouseFrame[0] = 0; }

		// 右クリック (MOUSE_INPUT_RIGHT = 2)
		if (tempMouse & MOUSE_INPUT_RIGHT) { _MouseFrame[1]++; }
		else { _MouseFrame[1] = 0; }

		// 中クリック (MOUSE_INPUT_MIDDLE = 4) もついでに対応しておくと楽です
		if (tempMouse & MOUSE_INPUT_MIDDLE) { _MouseFrame[2]++; }
		else { _MouseFrame[2] = 0; }

		// マウス座標もついでに更新
		DxLib::GetMousePoint(&_posX, &_posY);
	}
	/// <summary>
	/// Mouseが押された瞬間を取得する。	
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="MouseCode">DxlibのMouseCode</param>
	static inline bool isDownMouse(int MouseCode)
	{
		int index = GetIndex(MouseCode);
		return (index != -1) ? (_MouseFrame[index] == 1) : false;
	}
	/// <summary>
	/// Mouseが押され続けているかを取得する。
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="MouseCode">DxlibのMouseCode</param>
	static inline bool isHoldMouse(int MouseCode)
	{
		int index = GetIndex(MouseCode);
		return (index != -1) ? (_MouseFrame[index] >= 1) : false;
	}
	/// <summary>
	/// Mouseが押されているフレーム数を取得する.
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="MouseCode">DxlibのMouseCode</param>
	static inline int HoldMouseTime(int MouseCode)
	{
		int index = GetIndex(MouseCode);
		return (index != -1 && _MouseFrame[index] >= 1) ? _MouseFrame[index] : 0;
	}
	// 座標取得用の関数
	static inline int GetX() { return _posX; }
	static inline int GetY() { return _posY; }
private:
	// 内部用：マウスコードから配列インデックスへの変換
	static inline int GetIndex(int MouseCode)
	{
		if (MouseCode == MOUSE_INPUT_LEFT) return 0;
		if (MouseCode == MOUSE_INPUT_RIGHT) return 1;
		if (MouseCode == MOUSE_INPUT_MIDDLE) return 2;
		return -1;
	}
	/*staticメンバ変数*/
	//それぞれのMouseのFrame数
	inline static int _MouseFrame[3] = { 0 };
	//Mouseの座標
	inline static int _posX = 0;
	inline static int _posY = 0;
};