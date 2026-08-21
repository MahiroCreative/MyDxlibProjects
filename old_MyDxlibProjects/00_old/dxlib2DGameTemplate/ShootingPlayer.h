#pragma once
//STL.
#include <memory>
#include <vector>
//origin.
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"
#include "SimpleBullet.h"

// 型エイリアスを定義
using BulletPtr = std::unique_ptr<SimpleBullet>;
using BulletVector = std::vector<BulletPtr>;


class ShootingPlayer : public _baseGameObject2D
{
public:
	/*コンストラクタデストラクタ*/
	//コンストラクタ
	ShootingPlayer():
		_transform(),
		_rigidbody(),
		_collision(),
		_color(),
		_hp(),
		_moveSpeed(),
		_tShotFrame(),
		_tChargeFrame(),
		_tDamageFrame(),
		_isShot(),
		_isChargeShot()
	{
		//初期化
		Init();
	}
	//デストラクタ
	~ShootingPlayer() = default;

	/*定型メンバ関数*/
	//初期化
	void Init() override;
	//初期化(オーバーロード)
	void Init(Vector2 pos);
	//初期化(オーバーロード)
	void Init(Vector2 pos, float moveSpeed);
	//更新
	void Update() override;
	//描画
	void Draw() override;

	/*メンバ関数*/
	//移動
	void Move();
	//速度の更新
	void VelocityUpdate();
	//Playerそのものの描画
	void PlayerDraw();
	//残像描画
	void ShadowDraw();
	//チャージEffectの描画
	void EffectDraw();
	//弾の発射フラグの更新
	void ShotFlagUpdate();
	//チャージショットの発射フラグの更新
	void ChargeShotFlagUpdate();
	//コリジョンの更新
	void CollisionUpdate();
	//Damage処理
	void Damage(int damage);
	//無敵時間の更新
	void DamageUpdate();

	/*Debug用関数*/
	//コリジョン描画
	void DebugDraw();

	/*プロパティ*/
	//変数Getter.
	Transform2D GetTransform() { return _transform; }
	CircleCollision2D GetCollision() { return _collision; }
	int GetHp() { return _hp; }
	bool IsShot() { return _isShot; }
	bool IsChargeShot() { return _isChargeShot; }
	//定数Getter.
	int GetShotSize() const;
	int GetChargeShotSize() const;
	int GetShotSpeed() const;
	int GetChargeShotSpeed() const;
	int GetShotDamage() const;
	int GetChargeShotDamage() const;

private:
	/*メンバ変数*/
	//位置・倍率・回転
	Transform2D _transform;
	//速度・加速度
	Rigidbody2D _rigidbody;
	//Collision
	CircleCollision2D _collision;
	//色
	unsigned int _color;
	//HP
	int _hp;
	//移動速度
	float _moveSpeed;

	//タイマ変数
	//shotのtimer(前回のショットから経過したFrame)
	int _tShotFrame;
	//ChargeFrame(チャージ時間)
	int _tChargeFrame;
	//ダメージを受けてからの時間
	int _tDamageFrame;

	//フラグ変数
	//shotのフラグ
	bool _isShot;
	//ChargeShotのフラグ
	bool _isChargeShot;

};