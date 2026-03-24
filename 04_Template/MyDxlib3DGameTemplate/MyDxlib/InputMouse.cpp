#include "InputMouse.h"

void InputMouse::Update()
{
	//Dxlibの昨日でMouse入力を取得
	int tempMouse = DxLib::GetMouseInput();

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

bool InputMouse::isDownMouse(int MouseCode)
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

bool InputMouse::isHoldMouse(int MouseCode)
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

int InputMouse::HoldMouseTime(int MouseCode)
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

//静的メンバ変数の実態作成
int InputMouse::MouseFrame[2];