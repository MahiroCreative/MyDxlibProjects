#pragma once
//STL.
#include <memory>
//origin.
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"
#include "PlatformPlayer.h"


class PlatformScene : public _baseGameScene
{
public:
	/*コンストラクタデストラクタ*/ 
	//コンストラクタ
	PlatformScene()
		: _nextScene()
	{
		Init();
	}
	//デストラクタ
	~PlatformScene() = default;

	/*定型メンバ関数*/
	//初期化
	void Init() override;
	//更新
	int Update() override;
	//描画
	void Draw() override;
private:
	/*処理変数*/
	//nextScene.
	GameSetting::SceneState _nextScene;

	/*オブジェクト変数*/
	//型エイリアス
	using  PlayerPtr = std::shared_ptr<PlatformPlayer>;
	//Player.
	PlayerPtr _player;
};

