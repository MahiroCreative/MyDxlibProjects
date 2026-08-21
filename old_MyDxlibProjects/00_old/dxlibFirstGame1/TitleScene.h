#pragma once
#include "GameCommon.h"
#include "DxLib.h"
#include "MyGameLib.h"

class TitleScene
{
private:
	SceneKind _nextScene, _tempScene;//シーン遷移用
	int _arrowY;
	int _countFrame;//タイマ用
public:
	TitleScene() :
		_nextScene(SceneKind::TITLESCENE),
		_tempScene(SceneKind::TITLESCENE),
		_arrowY(480),
		_countFrame(0)
	{
	};
	/*メンバ関数*/
	SceneKind Update()
	{
		/*矢印操作*/
		//up.
		if(MyKeyInput::isDownKey(KEY_INPUT_UP))
		{
			if (_arrowY <= 480)
			{
				_arrowY = 560;//現在地が一番上なら一番下に移動
			}
			else
			{
				_arrowY -= 40;//40up.
			}

		}
		//down.
		if (MyKeyInput::isDownKey(KEY_INPUT_DOWN))
		{
			if (_arrowY >= 560)
			{
				_arrowY = 480;//現在地が一番下なら一番上に移動
			}
			else
			{
				_arrowY += 40;//40down.
			}
		}

		/*決定処理*/
		if (MyKeyInput::isDownKey(KEY_INPUT_RETURN))
		{
			if (_arrowY == 480)
			{
				_nextScene = SceneKind::GAMESCENE;
			}
			else if (_arrowY == 520)
			{
				_nextScene = SceneKind::RANKING;
			}
			else if (_arrowY == 560)
			{
				_nextScene = SceneKind::GAMEEND;
			}
		}

		/*タイマ更新*/
		_countFrame++;
		if (_countFrame > 80000) { _countFrame = 0; }//一定以上数が増えたら初期化(数は適当)

		return _nextScene;
	}
	void Draw()
	{
		//タイトル表示
		SetFontSize(80);//フォントサイズ上昇
		DrawString(360, 240, "DxlibFirstGame", GetColor(255, 255, 255));

		//ボタン表示
		SetFontSize(20);//フォントサイズをデフォルトに戻す
		DrawString(600, 480, "GameStart", GetColor(255, 255, 255));
		DrawString(600, 520, "Ranking", GetColor(255, 255, 255));
		DrawString(600, 560, "END", GetColor(255, 255, 255));

		//矢印表示(点滅させる)
		if ((_countFrame % 60) < 32 )
		{
			DrawString(560, _arrowY, "=>", GetColor(255, 255, 255));
		}

	}
	/// <summary>
	/// インスタンスの初期化
	/// </summary>
	void Init()
	{
		_nextScene = SceneKind::TITLESCENE;
		_tempScene = SceneKind::TITLESCENE;
		_arrowY = 480;
		_countFrame = 0;
	}
};