#include "SimpleBullet.h"

namespace {
	//初期値
	const Vector2 kDefaultPosition = Vector2(0.0f, 0.0f);
	const Vector2 kDefaultVelocity = Vector2::Right;
	constexpr int kDefaultRadius = 10;
	const unsigned int kDefaultCollisionColor = Color::RedColor;
	const float kDefaultMoveSpeed = 2.0f;
	const unsigned int kDefaultBulletColor = Color::OrangeColor;
	constexpr int kDefaultSize = 4;
	const std::string kDefaultTag = "";
}

void SimpleBullet::Init()
{
	//初期化
	_transform.Position = kDefaultPosition;
	_rigidbody.Velocity = kDefaultVelocity;
	_collision.Center = _transform.Position;
	_collision.Radius = kDefaultRadius;
	_collision.Color = kDefaultCollisionColor;
	_moveSpeed = kDefaultMoveSpeed;
	_color = kDefaultBulletColor;
	_size = kDefaultSize;
	_tag = kDefaultTag;
}

void SimpleBullet::Init(Vector2 pos)
{
	Init();
	_transform.Position = pos;
}

void SimpleBullet::Init(Vector2 pos, float moveSpeed)
{
	Init(pos);
	_moveSpeed = moveSpeed;
}

void SimpleBullet::Init(Vector2 pos, float moveSpeed, int size)
{
	Init(pos, moveSpeed);
	_size = size;
	_collision.Radius = size;
}

void SimpleBullet::Init(Vector2 pos, float moveSpeed, int size, std::string tag)
{
	Init(pos, moveSpeed, size);
	_tag = tag;
}

void SimpleBullet::Init(Vector2 pos, float moveSpeed, int size, std::string tag, unsigned int color)
{
	Init(pos, moveSpeed, size, tag);
	_color = color;
}

void SimpleBullet::Update()
{
	//移動
	Move();

	//コリジョンの更新
	CollisionUpdate();
}

void SimpleBullet::Draw()
{
	//描画
	DrawCircle(static_cast<int>(_transform.Position.X), static_cast<int>(_transform.Position.Y), _size, _color, TRUE);
}

void SimpleBullet::Move()
{
	//移動
	_transform.Position += _rigidbody.Velocity * _moveSpeed;

	//加速
	_rigidbody.Velocity += _rigidbody.Acceleration;
}

void SimpleBullet::CollisionUpdate()
{
	//コリジョンの中心位置更新
	_collision.Center = _transform.Position;
}

bool SimpleBullet::IsOutOfScreen()
{
	//画面外判定
	bool isOutLeft = _transform.Position.X <= 0;//左
	bool isOutRight = _transform.Position.X >= GameSetting::WINDOW_WIDTH;//右

	//画面外ならtrue
	if (isOutLeft || isOutRight) { return true; }

	//画面外でないならfalse
	return false;
}
