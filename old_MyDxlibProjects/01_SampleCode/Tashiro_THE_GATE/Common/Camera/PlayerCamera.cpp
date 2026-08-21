#include "PlayerCamera.h"
#include "Input.h"
#include "CommandList.h"
#include "Game.h"
#include "MathHelp.h"
#include "Object/Gate/Gate.h"
#include "Collider/ColliderCapsule.h"

PlayerCamera::PlayerCamera()
{
}

PlayerCamera::~PlayerCamera()
{
}

void PlayerCamera::Update(const Vec3& playerPos)
{
	constexpr float ANGLE_CAMERA = 1.0f;
	// カメラの回転
	auto& input = Input::GetInstance();
	const auto& trigger = input.GetTriggerData().RStick;
	m_info.targetPos = playerPos;
	// 左右回転
	auto rot = Quaternion::AngleAxis(trigger.x, Vec3::Up());
	m_info.front = rot * m_info.front;
	m_info.right = rot * m_info.right;
	// 上下回転
	m_info.vertexAngle = std::min<float>(std::max<float>(m_info.vertexAngle + trigger.y * -0.02f, -Game::PI_HALF_F), Game::PI_HALF_F);
	
	// FPS、TPS切り替え
	if (input.IsTriggerd(Command::BTN_CHANGE_VIEW))
	{
		m_info.isTps = !m_info.isTps;
	}
}

void PlayerCamera::OnWarp(const Vec3& preVelDir, const Vec3& newVelDir, const Vec3& pos)
{
	const auto& rotBase = Quaternion::GetQuaternion(preVelDir, newVelDir);
	const auto& euler = Quaternion::GetEuler(rotBase);
	const auto& rot = Quaternion::AngleAxis(euler.t1.y, Vec3::Up());
	m_info.front = rot * m_info.front;
	m_info.right = rot * m_info.right;
	m_info.targetPos = pos;

	auto newPos = m_info.targetPos - m_info.front * GetDistance();
#ifdef USE_CAMERA_COLLIDABLE
	m_rigid.SetPos(newPos);
	GetColliderData(0)->isTrigger = true;
#else
	m_info.cameraPos = newPos;
#endif
}
