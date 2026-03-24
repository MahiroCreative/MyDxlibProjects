#pragma once
#include "DxLib.h"
#include <math.h>

/*数学系*/
/// <summary>
/// 【ユーザ定義】【依存:無し】
/// int成分のベクター型.
/// </summary>
struct IntVector
{
public:
	/*メンバ変数*/
	int X, Y, Z;

	/*コンストラクタ*/
	IntVector() :X(0), Y(0), Z(0) {}//0ベクトル
	IntVector(int x, int y) :X(x), Y(y), Z(0) {}//2次元ベクトル
	IntVector(int x, int y, int z) : X(x), Y(y), Z(z) {}//3次元ベクトル

	/*メンバ関数*/
	//ベクトルの大きさを出す(int型)
	int Length()
	{
		return (int)sqrt(X * X + Y * Y + Z * Z);
	}

	/*演算子オーバーロード*/
	//Add.
	IntVector& operator+=(const IntVector& right)
	{
		X += right.X;
		Y += right.Y;
		Z += right.Z;
		return *this;
	}
	IntVector& operator+(const IntVector& right)
	{
		IntVector temp(*this);//演算結果用
		temp += right;
		return temp;
	}
	//Sub.
	IntVector& operator-=(const IntVector& right)
	{
		X -= right.X;
		Y -= right.Y;
		Z -= right.Z;
		return *this;
	}
	IntVector& operator-(const IntVector& right)
	{
		IntVector temp(*this);//演算結果用
		temp -= right;
		return temp;
	}
	//Mult.
	IntVector& operator*(const int& right)
	{
		IntVector temp(*this);//演算結果用
		temp.X = X * right;
		temp.Y = Y * right;
		temp.Z = Z * right;
		return temp;
	}
	//Div.
	IntVector& operator/(const int& right)
	{
		IntVector temp(*this);//演算結果用
		temp.X = X / right;
		temp.Y = Y / right;
		temp.Z = Z / right;
		return temp;
	}
};

/*入出力系*/
/// <summary>
/// 【ユーザ定義:static】【依存:Dxlib】
/// Keybordのみ。
/// </summary>
class MyKeyInput
{
private:
	/*staticメンバ変数*/
	//それぞれのKeyの入力フレーム数
	static int KeyFrame[];//定義だけ行い実態はグローバルで生成。
public:
	/*staticメンバ関数*/
	/// <summary>
	/// Keyの入力状態の更新。
	/// 毎フレームごとに実行することで更新する。
	/// </summary>
	static void Update()
	{
		// 現在のキーの入力状態
		char tmpKey[256];
		// 全てのキーの入力状態を取得
		GetHitKeyStateAll(tmpKey);
		//全てのKeyの入力状態の確認
		for (int i = 0; i < 256; i++)
		{
			//i番のKeyが押されているかを判定
			if (tmpKey[i] != 0)
			{
				//押しているKeyのフレーム数加算
				KeyFrame[i]++;
			}
			else
			{
				//押されていないKeyのフレーム数初期化
				KeyFrame[i] = 0;
			}
		}
	}
	/// <summary>
	/// Keyが押された瞬間を取得する。
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="KeyCode">DxlibのKeyCode</param>
	static bool isDownKey(int KeyCode)
	{
		if (KeyFrame[KeyCode] == 1)
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
	static bool isHoldKey(int KeyCode)
	{
		if (KeyFrame[KeyCode] >= 1)
		{
			return true;
		}
		return false;
	}
	/// <summary>
	/// Keyが押されているフレーム数を取得する
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <returns>DxlibのKeyCode</returns>
	static int HoldKeyTime(int KeyCode)
	{
		if (KeyFrame[KeyCode] >= 1)
		{
			return KeyFrame[KeyCode];
		}
		return 0;
	}
};
//静的メンバ変数の実態作成
int MyKeyInput::KeyFrame[256];
/// <summary>
/// 【ユーザ定義:static】【依存:Dxlib】
/// マウス
/// </summary>
class MyMouseInput
{
private:
	/*staticメンバ変数*/
	//それぞれのMouseのFrame数
	static int MouseFrame[];//定義だけ行い実態は下で生成。
public:
	/*staticメンバ関数*/
	/// <summary>
	/// Mouseの入力状態の更新。
	/// 毎フレームごとに実行することで更新する。
	/// </summary>
	static void Update()
	{
		int tempMouse = GetMouseInput();

		//左クリック
		if (tempMouse == MOUSE_INPUT_LEFT)
		{
			//押されている
			MouseFrame[0]++;
		}
		else
		{
			//押されていない
			MouseFrame[0] = 0;
		}
		//右クリック
		if (tempMouse == MOUSE_INPUT_RIGHT)
		{
			//押されている
			MouseFrame[1]++;
		}
		else
		{
			//押されていない
			MouseFrame[1] = 0;
		}
	}
	/// <summary>
	/// Mouseが押された瞬間を取得する。	
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="MouseCode">DxlibのMouseCode</param>
	static bool isDownKey(int MouseCode)
	{
		//左クリックの確認
		if ((MouseFrame[0] == 1) && (MOUSE_INPUT_LEFT == MouseCode))
		{
			return true;
		}
		//右クリックの確認
		if ((MouseFrame[1] == 1) && (MOUSE_INPUT_RIGHT == MouseCode))
		{
			return true;
		}
		return false;
	}
	/// <summary>
	/// マウスが押され続けているかを取得する。
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="MouseCode"></param>
	/// <returns></returns>
	static bool isHoldKey(int MouseCode)
	{
		//左クリックの確認
		if ((MouseFrame[0] >= 1) && (MOUSE_INPUT_LEFT == MouseCode))
		{
			return true;
		}
		//右クリックの確認
		if ((MouseFrame[1] >= 1) && (MOUSE_INPUT_RIGHT == MouseCode))
		{
			return true;
		}
		return false;
	}
	/// <summary>
	/// Keyが押されているフレーム数を取得する.
	/// Update()を毎フレーム行っている前提の関数。
	/// </summary>
	/// <param name="MouseCode"></param>
	/// <returns></returns>
	static int HoldKeyTime(int MouseCode)
	{
		//左クリックの確認
		if ((MouseFrame[0] >= 1) && (MOUSE_INPUT_LEFT == MouseCode))
		{
			return MouseFrame[0];
		}
		//右クリックの確認
		if ((MouseFrame[1] >= 1) && (MOUSE_INPUT_RIGHT == MouseCode))
		{
			return MouseFrame[1];
		}
		return 0;
	}
};
//静的メンバ変数の実態作成
int MyMouseInput::MouseFrame[2];