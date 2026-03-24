#include "Camera.h"
#include"Pad.h"
#include <math.h>


namespace
{
	constexpr float kCameraDist = 96;
	constexpr float kCameraHeight = 32;

	constexpr float kCameraNear = 1.f;
	constexpr float kCameraFar =100000.0f;

	//constexpr VECTOR kFirstPos=VGet(300, 300, -200);

	constexpr float kCameraFOV = 60.0f;

	constexpr float kcameraRotateSpeed = 0.05f;

	constexpr float kOneRapAngle = 360.f;
	constexpr float kWatchThisTime = 120.f;
}

Camera::Camera():
	m_pitchAngle(0),
	m_watchCount(0),
	m_isFirstPerson(0),
	m_isBoost(false),
	m_upVec(0,1,0),
	m_frontVec(0,0,1)
{
	
	m_cameraUpdate = &Camera::NeutralUpdate;
	//奥行0.1～1000までをカメラの描画範囲とする
	
	// FOV(視野角)を60度に
	SetupCamera_Perspective(kCameraFOV * (static_cast<float>(DX_PI_F) / 180.0f));

	m_pos = { 0.0f,100.0f, -200.0f };
	m_playerToCameraVec = { 0.f,100.f,-200.f };
	m_postLookPointPos = { 0,0,0 };
	m_fowardVec = { 0.f,0.f,0.1f };

	m_lightHandle = CreateSpotLightHandle(m_pos.VGet(),GetCameraFrontVector() ,DX_PI_F*2,
		DX_PI_F*2,
		2000.0f,
		0.0f,
		0.002f,
		0.0f);
}

Camera::~Camera()
{
	// 処理なし.
	DeleteLightHandle(m_lightHandle);
}

void Camera::Update(Vec3 LookPoint)
{
	if (m_isBoost)
	{
		// FOV(視野角)を60度に
		SetupCamera_Perspective(kCameraFOV * (static_cast<float>(DX_PI_F)*2 / 180.0f));

	}
	else
	{
		// FOV(視野角)を60度に
		SetupCamera_Perspective(kCameraFOV * (static_cast<float>(DX_PI_F) / 180.0f));

	}
	SetCameraNearFar(kCameraNear, kCameraFar);
	(this->*m_cameraUpdate)(LookPoint);
}

void Camera::SetCamera(Vec3 LookPoint)
{
	m_lookPoint = LookPoint;
	SetLightPositionHandle(m_lightHandle, Vec3(LookPoint + m_upVec * 120).VGet());
	SetLightDirectionHandle(m_lightHandle, (Vec3(GetCameraUpVector()) * -1).VGet());

	Vec3 velocity;
	velocity.x = (m_cameraPoint.x - m_pos.x) / 15.f;
	velocity.y = (m_cameraPoint.y - m_pos.y) / 15.f;
	velocity.z = (m_cameraPoint.z - m_pos.z) / 15.f;
	m_pos += velocity;//イージング

	SetCameraPositionAndTargetAndUpVec(m_pos.VGet(), Vec3(m_lookPoint + m_upVec).VGet(), m_upVec.VGet());
	m_postLookPointPos = m_lookPoint;
}

void Camera::SetAimCamera(Vec3 LookPoint)
{
	m_lookPoint = LookPoint;
	SetLightPositionHandle(m_lightHandle, Vec3(LookPoint + m_upVec * 120).VGet());
	SetLightDirectionHandle(m_lightHandle, (Vec3(GetCameraUpVector()) * -1).VGet());

	m_pos = m_cameraPoint;

	DrawSphere3D(m_pos.VGet(), 20, 8, 0xffffff, 0xffffff, true);

	SetCameraPositionAndTargetAndUpVec(m_pos.VGet(),Vec3(m_pos+LookPoint).VGet(), m_upVec.VGet());
	//SetCameraPositionAndTargetAndUpVec(VGet(0,400,-500), VGet(0,0,0), m_upVec.VGet());

	m_postLookPointPos = m_lookPoint+m_pos;
}

void Camera::DebagDraw()
{
}

Vec3 Camera::cameraToPlayer(const Vec3& targetPos)
{
	Vec3 cameraToPlayer (sqrt((targetPos.x - m_pos.x) * (targetPos.x - m_pos.x)), 0.0f, sqrt((targetPos.z - m_pos.z) * (targetPos.z - m_pos.z)) );
	return cameraToPlayer;
}

void Camera::SetCameraPos(Vec3 LookPoint)
{
}

void Camera::NeutralUpdate(Vec3 LookPoint)
{
	if (Pad::IsTrigger(PAD_INPUT_Y))//XBoxコントローラーのL
	{
		m_cameraUpdate = &Camera::AimingUpdate;
	}
	SetCamera(LookPoint);
}

void Camera::AimingUpdate(Vec3 LookPoint)
{
	if (Pad::IsTrigger(PAD_INPUT_Y))//XBoxコントローラーのL
	{
		m_cameraUpdate = &Camera::NeutralUpdate;
	}
	SetAimCamera(LookPoint);
}

void Camera::WatchThisUpdate(Vec3 LookPoint)
{
	m_watchCount++;

	SetLightPositionHandle(m_lightHandle, m_pos.VGet());
	SetLightDirectionHandle(m_lightHandle, GetCameraFrontVector());
	Vec3 velocity;
	velocity.x = (m_cameraPoint.x - m_pos.x) / 10.f;
	velocity.y = (m_cameraPoint.y - m_pos.y) / 10.f;
	velocity.z = (m_cameraPoint.z - m_pos.z) / 10.f;
	m_pos += velocity;//イージング
	
	SetCameraPositionAndTargetAndUpVec(m_pos.VGet(),Vec3(m_lookPoint + m_upVec.GetNormalized() * 100.0f).VGet(), m_upVec.VGet());
	m_postLookPointPos = m_lookPoint;
	if (m_watchCount > kWatchThisTime)
	{
		m_watchCount = 0;
		if(m_isFirstPerson)m_cameraUpdate = &Camera::SetCameraFirstPersonPos;
		else { m_cameraUpdate = &Camera::NeutralUpdate; }
	}
}

void Camera::SetCameraFirstPersonPos(Vec3 LookPoint)
{
	SetLightPositionHandle(m_lightHandle, Vec3(LookPoint + m_upVec * 120).VGet());
	SetLightDirectionHandle(m_lightHandle, (Vec3(GetCameraUpVector()) * -1).VGet());

	if (Pad::IsTrigger(PAD_INPUT_Y))//XBoxコントローラーのL
	{
		m_cameraUpdate = &Camera::NeutralUpdate;
	}
	Vec3 target = LookPoint;
	SetCameraPositionAndTargetAndUpVec((LookPoint+m_upVec*300).VGet(), target.VGet(), GetCameraUpVector());
}

void Camera::SetCameraThirdPersonPos(Vec3 LookPoint)
{
	SetLightPositionHandle(m_lightHandle, Vec3(LookPoint + m_upVec * 120).VGet());
	SetLightDirectionHandle(m_lightHandle, (Vec3(GetCameraUpVector()) * -1).VGet());

	SetCameraPositionAndTargetAndUpVec(m_pos.VGet(), LookPoint.VGet(), m_upVec.VGet());
}

void Camera::SetDebugCameraPoint()
{
	//SetCameraPositionAndTargetAndUpVec(VGet(0, 400, -500), VGet(0, 0, 0), m_upVec.VGet());
}

void Camera::WatchThis(Vec3 lookpoint, Vec3 cameraPos, Vec3 upVec)
{
	m_cameraUpdate = &Camera::WatchThisUpdate;
	m_lookPoint = lookpoint;
	m_cameraPoint = cameraPos;
	m_upVec = upVec;
	WatchThisUpdate(lookpoint);
}
