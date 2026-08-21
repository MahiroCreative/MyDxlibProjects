#include "GateCamera.h"
#include "Game.h"
#include "Object/Gate/Gate.h"

GateCamera::GateCamera()
{
	m_info.isTps = false;
}

GateCamera::~GateCamera()
{
}

void GateCamera::Update(const Vec3& dir, const Vec3& playerCameraFront)
{
	// プレイヤーのカメラの上下方向の回転度合いを調べる
	auto atan2 = std::atan2(playerCameraFront.y, std::sqrt(playerCameraFront.x * playerCameraFront.x + playerCameraFront.z * playerCameraFront.z)) * -1;
	// 度数法に変換
	auto angle = atan2 * Game::RAD_2_DEG;
	// 軸の計算
	auto axis = Vec3::Cross(dir, m_baseViewDir);
	// 正面を上分回転させる
	m_info.front = Quaternion::AngleAxis(angle, axis) * m_baseViewDir;
}
