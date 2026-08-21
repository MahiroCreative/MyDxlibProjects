#pragma once
#include "Camera.h"

class Gate;

class PlayerCamera : public Camera
{
public:
	PlayerCamera();
	~PlayerCamera();

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="playerPos">プレイヤーの座標</param>
	void Update(const Vec3& playerPos);

	/// <summary>
	/// ワープ処理
	/// </summary>
	/// <param name="playerPos">プレイヤーの座標</param>
	/// <param name="gate">ワープ元のゲート</param>
	/// <param name="pairGate">ワープ先のゲート</param>
	void OnWarp(const Vec3& preVelDir, const Vec3& newVelDir, const Vec3& pos);
};

