#include "PlatformPlayer.h"

namespace
{
	//物理挙動
	const Vector2 _kGravity{ 0,0.5f };//重力加速度
	constexpr float _kAcceleration = 0.5f;//加速度
	constexpr float _kFriction = 0.1f;//摩擦係数
	constexpr float _kMaxHorizontalVelocity = 6;//最大水平速度
	constexpr float _kMaxVerticalVelocity = 16;//最大垂直速度
	constexpr float _kSpeedPower = 0.5f;//移動力(調整変数)
	constexpr float _kJumpPower = 8;//ジャンプ力(調整変数)
	//プレイヤーのサイズ
	constexpr int _kPlayerWidth = 24;//幅
	constexpr int _kPlayerHeight = 32;//高さ
	//プレイヤーの描画
	const int _kPlayerColor = Color::YellowColor;//色
	//その他
	constexpr float _kfloorY = 608;//地面の位置
}

void PlatformPlayer::Init()
{
	//変数初期化
	_transform.Position = Vector2(10, 100);//位置
	_rigidbody.Velocity = Vector2(0, 0);//速度
	_rigidbody.Acceleration = Vector2(0, 0);//加速度
}

void PlatformPlayer::Update()
{
	//条件チェック
	//状態の確認
	bool isGround = (_transform.Position.Y >= _kfloorY);//地面にいるかどうか
	//入力値の取得
	bool inputLeft = InputKey::isHoldKey(KEY_INPUT_A);//左入力
	bool inputRight = InputKey::isHoldKey(KEY_INPUT_D);//右入力
	bool inputJump = InputKey::isDownKey(KEY_INPUT_SPACE) && isGround;//ジャンプ入力
	bool inputNothing = !(inputLeft || inputRight || inputJump);//何も入力されていない

	//入力値の取得
	Vector2 inputDir{ 0,0 };//一時変数
	if (inputLeft) { inputDir.X =  -_kSpeedPower; }//左入力
	if (inputRight) { inputDir.X = _kSpeedPower; }//右入力
	if (inputJump) { inputDir.Y = -_kJumpPower; }//ジャンプ入力
	if (inputNothing) { inputDir = Vector2::Zero;}//何も入力されていない

	//加速度の更新
	_rigidbody.Acceleration = _kGravity + inputDir;

	//摩擦の計算
	_rigidbody.Acceleration.X -= _rigidbody.Velocity.X * _kFriction;

	//速度の更新
	_rigidbody.Velocity += _rigidbody.Acceleration;

	//速度制限
	// 右方向
	if (_rigidbody.Velocity.X >= _kMaxHorizontalVelocity) { _rigidbody.Velocity.X = _kMaxHorizontalVelocity; }
	// 左方向
	if (_rigidbody.Velocity.X <= -_kMaxHorizontalVelocity) { _rigidbody.Velocity.X = -_kMaxHorizontalVelocity; }
	//垂直方向
	if (_rigidbody.Velocity.Y >= _kMaxVerticalVelocity) { _rigidbody.Velocity.Y = _kMaxVerticalVelocity; }

	//地面にいて、ジャンプしていないならなら水平方向の速度を消す
	if (isGround && !inputJump) { _rigidbody.Velocity.Y = 0; }

	//位置の更新
	_transform.Position += _rigidbody.Velocity;

	//位置の補正
	if (_transform.Position.Y >= _kfloorY) { _transform.Position.Y = _kfloorY; }//地面にいる場合
}

void PlatformPlayer::Draw()
{
	//描画位置の決定
	int PlayerPosX = _transform.Position.X;//X座標
	int PlayerPosY = _transform.Position.Y;//Y座標
	int DrawRightX = PlayerPosX + _kPlayerWidth;//右端のX座標
	int DrawRightY = PlayerPosY + _kPlayerHeight;//下端のY座標
	//四角形描画
	DrawBox(PlayerPosX, PlayerPosY, DrawRightX, DrawRightY, _kPlayerColor, TRUE);
}


