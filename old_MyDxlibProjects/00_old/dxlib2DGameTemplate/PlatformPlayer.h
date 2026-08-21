#pragma once
//STL.
#include <memory>
//origin.
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"

class PlatformPlayer : public _baseGameObject2D
{
public:
	//コンストラクタとデストラクタ
	PlatformPlayer() : _transform(), _rigidbody()
	{
		Init();
	}
	~PlatformPlayer() = default;

	/*定型メンバ関数*/
	//初期化
	void Init() override;
	//更新
	void Update() override;
	//描画
	void Draw() override;

	/*メンバ関数*/

private:
	//メンバ変数
	Transform2D _transform;
	Rigidbody2D _rigidbody;
};

