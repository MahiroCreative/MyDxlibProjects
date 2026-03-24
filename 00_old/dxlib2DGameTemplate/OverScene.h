#pragma once
//STL.
#include <memory>
//origin.
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"

class OverScene : public _baseGameScene
{
public:
	/*コンストラクタデストラクタ*/
	//コンストラクタ
	OverScene()
		: _nextScene(), _overText()
	{
		Init();
	}
	//デストラクタ
	~OverScene() = default;
	/*定型メンバ関数*/
	//初期化
	void Init() override;
	//更新
	int Update() override;
	//描画
	void Draw() override;
private:
	/*メンバ変数*/
	//次のシーン
	GameSetting::SceneState _nextScene;
	//シーンロゴがわりの文字
	SimpleText _overText;
};