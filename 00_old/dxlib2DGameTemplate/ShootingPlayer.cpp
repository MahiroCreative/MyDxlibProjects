#include "ShootingPlayer.h"

/*定数*/
namespace
{
	//ショットのサイズ
	constexpr int _kShotSize = 4;
	//ショットのインターバル
	constexpr int _kShotInterval = 16;
	//ショットのダメージ
	constexpr int _kShotDamage = 1;
	//ショットの速度
	constexpr int _kShotSpeed = 8;
	//チャージショットのサイズ
	constexpr int _kChargeShotSize = 20;
	//チャージショットのインターバル
	constexpr int _kChargeShotInterval = 60;
	//チャージショットのダメージ
	constexpr int _kChargeShotDamage = 3;
	//チャージショットの速度
	constexpr int _kChargeShotSpeed = 12;
	//ショットキー
	constexpr int _kShotKey = KEY_INPUT_RETURN;
	//ダメージインターバル(無敵時間)
	constexpr int _kDamageInterval = 20;
}


/*定型メンバ関数*/
//初期化
void ShootingPlayer::Init()
{
	//変数初期化
	_transform.Position = Vector2(0, 0);//位置
	_rigidbody.Velocity = Vector2(0, 0);//速度
	_rigidbody.Acceleration = Vector2(0, 0);//加速度
	_collision.Center = _transform.Position;//中心
	_collision.Radius = 10;//コリジョンの半径
	_collision.Color = Color::RedColor;//色
	_color = Color::WhiteColor;//色
	_moveSpeed = 3.0f;//移動速度
	_tShotFrame = 0;//ショットフレーム
	_tChargeFrame = 0;//チャージフレーム
	_tDamageFrame = 0;//ダメージフレーム
	_isShot = false;//ショットフラグ
	_isChargeShot = false;//チャージショットフラグ
	_hp = 3;//HP
}
void ShootingPlayer::Init(Vector2 pos)
{
	//初期化
	Init();//初期化
	_transform.Position = pos;//位置
}
void ShootingPlayer::Init(Vector2 pos, float moveSpeed)
{
	//初期化
	Init();//初期化
	_transform.Position = pos;//位置
	_moveSpeed = moveSpeed;//移動速度
}

//更新
void ShootingPlayer::Update()
{
	//Key入力による速度の更新
	VelocityUpdate();

	//shotフラグの更新
	ShotFlagUpdate();

	//chargeShotフラグの更新
	ChargeShotFlagUpdate();

	//無敵時間の更新
	DamageUpdate();

	//移動
	Move();

	//コリジョンの位置更新
	CollisionUpdate();
}

//描画
void ShootingPlayer::Draw()
{
	//プレイヤーの描画
	PlayerDraw();

	//Debug描画
	DebugDraw();
}



//移動
void ShootingPlayer::Move()
{
	//移動
	_transform.Position += _rigidbody.Velocity * _moveSpeed;

	//加速
	_rigidbody.Velocity += _rigidbody.Acceleration;

	//ストップ
	if (_transform.Position.X <= 0) { _rigidbody.Velocity.X = 0; }//左端
	if (_transform.Position.X >= GameSetting::WINDOW_WIDTH) { _rigidbody.Velocity.X = 0; }//右端
	if (_transform.Position.Y <= 0) { _rigidbody.Velocity.Y = 0; }//上端
	if (_transform.Position.Y <= 0) { _rigidbody.Velocity.Y = 0; }//下端
}

//速度の更新
void ShootingPlayer::VelocityUpdate()
{
	//Key入力チェック
	bool isUP = InputKey::isHoldKey(KEY_INPUT_W);
	bool isDOWN = InputKey::isHoldKey(KEY_INPUT_S);
	bool isLEFT = InputKey::isHoldKey(KEY_INPUT_A);
	bool isRIGHT = InputKey::isHoldKey(KEY_INPUT_D);

	//Key入力による速度の更新
	if (isUP) { _rigidbody.Velocity.Y = -1; }//上
	if (isDOWN) { _rigidbody.Velocity.Y = 1; }//下
	if (isLEFT) { _rigidbody.Velocity.X = -1; }//左
	if (isRIGHT) { _rigidbody.Velocity.X = 1; }//右

	//何も押されていない場合
	if (!isUP && !isDOWN && !isLEFT && !isRIGHT) { _rigidbody.Velocity = Vector2::Zero; }

	//速度の正規化
	if (_rigidbody.Velocity.LengthSq() > 0) { _rigidbody.Velocity = _rigidbody.Velocity.Normalize(); }

}

//Playerそのものの描画
void ShootingPlayer::PlayerDraw()
{
	//残像描画
	ShadowDraw();

	//Player描画
	DrawCircle(static_cast<int>(_transform.Position.X), static_cast<int>(_transform.Position.Y), 10, _color, TRUE);

	//Effect描画
	EffectDraw();

}

//弾を撃つかどうか
void ShootingPlayer::ShadowDraw()
{
	//残像5
	DrawCircle(static_cast<int>(_transform.Position.X - _rigidbody.Velocity.X * 32), static_cast<int>(_transform.Position.Y - _rigidbody.Velocity.Y * 32), 1, GetColor(190, 190, 0), TRUE);
	//残像4
	DrawCircle(static_cast<int>(_transform.Position.X - _rigidbody.Velocity.X * 24), static_cast<int>(_transform.Position.Y - _rigidbody.Velocity.Y * 24), 2, GetColor(200, 200, 0), TRUE);
	//残像3
	DrawCircle(static_cast<int>(_transform.Position.X - _rigidbody.Velocity.X * 12), static_cast<int>(_transform.Position.Y - _rigidbody.Velocity.Y * 12), 4, GetColor(210, 210, 0), TRUE);
	//残像2
	DrawCircle(static_cast<int>(_transform.Position.X - _rigidbody.Velocity.X * 8), static_cast<int>(_transform.Position.Y - _rigidbody.Velocity.Y * 8), 6, GetColor(220, 220, 0), TRUE);
	//残像1
	DrawCircle(static_cast<int>(_transform.Position.X - _rigidbody.Velocity.X * 4), static_cast<int>(_transform.Position.Y - _rigidbody.Velocity.Y * 4), 8, GetColor(230, 230, 0), TRUE);
}

//チャージEffectの描画
void ShootingPlayer::EffectDraw()
{
	//チャージ演出
	if (_tChargeFrame > 8)
	{
		//最大の大きさ制限
		int tempValue = _tChargeFrame;
		if (tempValue > 100) { tempValue = 100; }
		//塵5
		DrawCircle(static_cast<int>(_transform.Position.X + _rigidbody.Velocity.X * 16), static_cast<int>(_transform.Position.Y + _rigidbody.Velocity.Y * 16), 0.04f * tempValue, GetColor(255, 5, 0), TRUE);
		//塵4
		DrawCircle(static_cast<int>(_transform.Position.X - _rigidbody.Velocity.X * 12), static_cast<int>(_transform.Position.Y + _rigidbody.Velocity.Y * 12), 0.08f * tempValue, GetColor(255, 45, 0), TRUE);
		//塵3
		DrawCircle(static_cast<int>(_transform.Position.X + _rigidbody.Velocity.X * 8), static_cast<int>(_transform.Position.Y - _rigidbody.Velocity.Y * 8), 0.12f * tempValue, GetColor(255, 85, 0), TRUE);
		//塵2
		DrawCircle(static_cast<int>(_transform.Position.X - _rigidbody.Velocity.X * 4), static_cast<int>(_transform.Position.Y - _rigidbody.Velocity.Y * 4), 0.16f * tempValue, GetColor(255, 125, 0), TRUE);
		//塵1
		DrawCircle(static_cast<int>(_transform.Position.X + _rigidbody.Velocity.X * 2), static_cast<int>(_transform.Position.Y + _rigidbody.Velocity.Y * 2), 0.20f * tempValue, GetColor(255, 165, 0), TRUE);
	}
}

void ShootingPlayer::ShotFlagUpdate()
{
	//ショット条件
	bool isShotFrame = (_tShotFrame > _kShotInterval);
	bool isDown = InputKey::isDownKey(_kShotKey);

	//ショットフラグの更新
	if (isShotFrame && isDown)
	{
		_isShot = true;
		_tShotFrame = 0;//ショットフレームのリセット
	}
	else
	{
		_isShot = false;
		_tShotFrame++;//ショットフレームの更新
	}
}

void ShootingPlayer::ChargeShotFlagUpdate()
{
	//チャージショット条件
	bool isChargeFrame = (_tChargeFrame > _kChargeShotInterval);
	bool isNotHold = !InputKey::isHoldKey(_kShotKey);

	//チャージショットフラグの更新
	if (isChargeFrame && isNotHold)
	{
		_isChargeShot = true;
		_tChargeFrame = 0;//チャージフレームのリセット
	}
	else
	{
		_isChargeShot = false;
	}
	
	//チャージフレームの更新
	if(InputKey::isHoldKey(_kShotKey))
	{
		_tChargeFrame++;
	}
	else
	{
		_tChargeFrame = 0;//チャージフレームのリセット
	}
}

void ShootingPlayer::CollisionUpdate()
{
	//コリジョンの中心位置更新
	_collision.Center = _transform.Position;
}

void ShootingPlayer::Damage(int damage)
{
	//ダメージ条件
	bool isDamage = (_tDamageFrame > _kDamageInterval);

	//ダメージを受ける
	if (isDamage)
	{
		//Hpを減らす
		_hp = _hp - damage; 
		//damageFrameのリセット
		_tDamageFrame = 0;
	}

	//色を変える
	_color = Color::RedColor;

}

void ShootingPlayer::DamageUpdate()
{
	//damageFrameの更新
	_tDamageFrame++;
	//damageFrameが無敵時間を超えたら色を戻す
	if (_tDamageFrame > _kDamageInterval) { _color = Color::WhiteColor; }
	//上昇しすぎたら戻す
	if (_tDamageFrame > 80) { _tDamageFrame = 80; }
}


/*Debug用*/
void ShootingPlayer::DebugDraw()
{
	//コリジョンの描画
	DrawCircle(static_cast<int>(_collision.Center.X), static_cast<int>(_collision.Center.Y), _collision.Radius, _collision.Color, FALSE);

	//Hpの描画
	DrawFormatString(0, 24, GetColor(255, 255, 255), "PlayerHP:%d", _hp);
}

/*プロパティ*/
//定数Getter.
int ShootingPlayer::GetShotSize() const
{
	return _kShotSize;
}
int ShootingPlayer::GetChargeShotSize() const
{
	return _kChargeShotSize;
}
int ShootingPlayer::GetShotSpeed() const
{
	return _kShotSpeed;
}
int ShootingPlayer::GetChargeShotSpeed() const
{
	return _kChargeShotSpeed;
}
int ShootingPlayer::GetShotDamage() const
{
	return _kShotDamage;
}
int ShootingPlayer::GetChargeShotDamage() const
{
	return _kChargeShotDamage;
}