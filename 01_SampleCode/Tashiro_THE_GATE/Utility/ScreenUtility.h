#pragma once
#include <DxLib.h>

class ScreenUtility
{
public:
	static int MakeDepthRT(int screenW, int screenH)
	{
		SetCreateGraphChannelBitDepth(32);
		SetCreateDrawValidGraphChannelNum(1);
		return MakeScreen(screenW, screenH, false);
	}
};

