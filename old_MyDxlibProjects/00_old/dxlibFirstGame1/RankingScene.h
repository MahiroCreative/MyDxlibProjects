#pragma once
#include "GameCommon.h"
#include "DxLib.h"
#include "MyGameLib.h"


class RankingScene
{
private:
	/*メンバ変数*/
	SceneKind _nextScene;//シーン遷移用
public:
	/*メンバ変数*/
	static int _maxScore;
	/*コンストラクタ*/
	RankingScene() :
		_nextScene(SceneKind::RANKING)
	{
	};
	/*メンバ関数*/
	//処理の更新
	SceneKind Update()
	{
		//Enterキーを押したら指定シーンに遷移
		if (MyKeyInput::isDownKey(KEY_INPUT_RETURN))
		{
			_nextScene = SceneKind::TITLESCENE;
		}

		return _nextScene;
	}
	void Draw()
	{
		//スコアの表示
		SetFontSize(80);//フォントサイズ上昇
		DrawFormatString(400, 240, GetColor(255, 255, 2555), "MaxScore:%d", _maxScore);
		SetFontSize(20);//フォントサイズをデフォルトに戻す
		DrawFormatString(480, 500, GetColor(255, 255, 255), "-- Return with enter --");

	}
	void DebugDraw()
	{
		//現在のシーン名を描画
		DrawString(8, 8, "SceneName:RankingScene", GetColor(0, 255, 255));
	}
	/// <summary>
	/// インスタンスの初期化
	/// </summary>
	void Init()
	{
		//メンバ変数の初期化
		_nextScene = SceneKind::RANKING;
	}

};
int RankingScene::_maxScore = 0;