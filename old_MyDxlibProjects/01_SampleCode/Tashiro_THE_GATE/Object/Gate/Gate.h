#pragma once
#include "Object/Object3DBase.h"
#include <memory>
#include "Collidable.h"
#include "GateManager.h"
#include "Vec3.h"
#include "Quaternion.h"

namespace MyEngine
{
	class ColliderCapsule;
}
class Player;
class FileBase;
class GateCamera;

class Gate : public Object3DBase
{
public:
	Gate(const std::shared_ptr<GateCamera>& camera, const std::shared_ptr<GateCamera>& cameraFromPair, GateKind kind);
	virtual ~Gate();

	void Init(ObjectTag hitObjTag, const Vec3& pos, const Vec3& norm, const Vec3& dir, const std::weak_ptr<Player>& player);
	void ChangePos(ObjectTag hitObjTag, const Vec3& pos, const Vec3& norm, const Vec3& dir);
	void SetCameraPos();
	void SetPairGate(const std::weak_ptr<Gate>& pairGate) { m_pairGate = pairGate; }

	void Update();
	void DrawGatePos() const;
	void DrawGate(int tex) const;

	bool CheckWarp(const Vec3& targetPos);
	void OnWarp(const Vec3& targetPos, MyEngine::Rigidbody& targetRigid, bool isAddGravity);

	GateKind GetKind() const { return m_kind; }
	ObjectTag GetHitObjectTag() const { return m_hitObjTag; }
	const std::shared_ptr<MyEngine::ColliderCapsule> GetCol() const { return m_collider; }
	const std::shared_ptr<Gate> GetPairGate() const { return m_pairGate.lock(); }
	const Vec3& GetNorm() const { return m_norm; }

private:
	void OpenUpdate();
	void NormalUpdate();
	void CloseUpdate();

private:
	using UpdateFunc_t = void(Gate::*)();
	UpdateFunc_t m_updateFunc;
	std::shared_ptr<GateCamera> m_camera;
	std::shared_ptr<GateCamera> m_cameraFromPair;
	std::shared_ptr<FileBase> m_dissolve;
	std::shared_ptr<FileBase> m_warpSe;
	std::shared_ptr<MyEngine::ColliderCapsule> m_collider;

	std::weak_ptr<Player> m_player;
	std::weak_ptr<Gate> m_pairGate;

	GateKind m_kind;

	ObjectTag m_hitObjTag;

	Vec3 m_norm;
	Vec3 m_right;

	int m_cbuffH;

private:
	struct UserData
	{
		Vec3 center;
		float radius;
		Vec3 dir;
		float size;
		float frame;
		bool isCreate;
		bool dummy1[3];
		float dummy2[2];
	};
	UserData* m_userData;
};
