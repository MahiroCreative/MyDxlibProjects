#include "CameraManager.h"
#include "Camera.h"
#include "PlayerCamera.h"
#include "GateCamera.h"

CameraManager::CameraManager()
{
	for (int i = 0; i < static_cast<int>(CameraKind::MAX); ++i)
	{
		auto& camera = m_cameras[i];
		if (i == static_cast<int>(CameraKind::PLAYER))
		{
			camera = std::make_shared<PlayerCamera>();
		}
		else
		{
			camera = std::make_shared<GateCamera>();
		}
	}
}

CameraManager::~CameraManager()
{
}

void CameraManager::Update()
{
	for (auto& item : m_cameras)
	{
		item->Update();
	}
}

void CameraManager::End()
{

#ifdef USE_CAMERA_COLLIDABLE
	for (auto& item : m_cameras)
	{
		item->End();
	}
#endif
}

void CameraManager::AppInfo(CameraKind kind)
{
	m_cameras[static_cast<size_t>(kind)]->AppInfo();
}

void CameraManager::OnEntity(CameraKind kind, const std::shared_ptr<GateManager>& gateMgr)
{
#ifdef USE_CAMERA_COLLIDABLE
	auto& item = m_cameras[static_cast<size_t>(kind)];
	item->OnEntity();
	item->SetGateManager(gateMgr);
#endif
}

