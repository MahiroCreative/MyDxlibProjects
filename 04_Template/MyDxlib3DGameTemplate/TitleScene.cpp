#pragma once
#include "TitleScene.h"
#include "GameSetting.h"

namespace
{
	//矢印の移動量
	constexpr float _kMoveArrow = 40.0f;
	//矢印の表示切替時間
	constexpr int _kArrowShowTime = 60;
	//矢印の非表示切替時間
	constexpr int _kArrowDeleteTime = 30;
	//矢印の初期位置
	const Vector3 _kArrowFirstPos = Vector3(GameSetting::WINDOW_CENTER_X - 120.0f, 280.0f);
	//1番目のテキストの表示位置
	const Vector3 _kFirstTextPos = Vector3(GameSetting::WINDOW_CENTER_X - 80.0f, 280.0f);
	//2番目のテキストの表示位置
	const Vector3 _kSecondTextPos = Vector3(GameSetting::WINDOW_CENTER_X - 80.0f, 320.0f);
	//3番目のテキストの表示位置
	const Vector3 _kThirdTextPos = Vector3(GameSetting::WINDOW_CENTER_X - 80.0f, 360.0f);

}

void TitleScene::Init()
{

}

void TitleScene::Update()
{

}

void TitleScene::Draw()
{
	
}
