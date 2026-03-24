#pragma once
//STL.
#include <memory>
//origin.
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"

/// <summary>
/// Simpleな弾クラス
/// 速度・色・サイズ・タグを持つ.
/// _baseGameObject2Dを継承
/// </summary>
class SimpleBullet : public _baseGameObject2D
{
public:
	/* コンストラクタ・デストラクタ */
	SimpleBullet() : _moveSpeed(0.0f), _color(0), _size(0), _tag("")
	{
		Init();
	}
	~SimpleBullet() override = default;

	/* 定型メンバ関数 */
	void Init() override;
	void Init(Vector2 pos);
	void Init(Vector2 pos, float moveSpeed);
	void Init(Vector2 pos, float moveSpeed, int size);
	void Init(Vector2 pos, float moveSpeed, int size, std::string tag);
	void Init(Vector2 pos, float moveSpeed, int size, std::string tag, unsigned int color);
	void Update() override;
	void Draw() override;

	/* メンバ関数 */
	//移動
	void Move();
	//Collisionの更新
	void CollisionUpdate();
	//画面外判定
	bool IsOutOfScreen();

	// getter
	Transform2D GetTransform() const { return _transform; }
	CircleCollision2D GetCollision() const { return _collision; }
	std::string GetTag() const { return _tag; }

private:
	/* メンバ変数 */
	Transform2D _transform;         // 位置・倍率・回転
	Rigidbody2D _rigidbody;         // 速度
	CircleCollision2D _collision;   // コリジョン
	float _moveSpeed;               // 移動速度
	unsigned int _color;            // 色
	int _size;                      // サイズ
	std::string _tag;               // タグ
};