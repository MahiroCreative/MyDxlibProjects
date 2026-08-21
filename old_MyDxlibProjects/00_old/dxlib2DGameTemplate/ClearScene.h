#pragma once
//STL.
#include <memory>
//origin.
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"


class ClearScene : public _baseGameScene
{
public:
	/*コンストラクタデストラクタ*/
	//コンストラクタ
	ClearScene()
		: _nextScene()
	{
		Init();
	}
	//デストラクタ
	~ClearScene() = default;
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
	SimpleText _clearText;
};