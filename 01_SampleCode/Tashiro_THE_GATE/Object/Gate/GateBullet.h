#pragma once
#include "Collidable.h"
#include <memory>

enum class GateKind;
class GateManager;

class GateBullet final : public MyEngine::Collidable
{
public:
	GateBullet(const std::shared_ptr<GateManager>& gateMgr, GateKind kind);
	~GateBullet();

	void Init(const Vec3& pos, const Vec3& dir);
	void Update();

	bool IsBreak() const { return m_isBreak; }
	const GateKind& GetKind() const { return m_kind; }

	virtual void OnCollideEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo) override;

private:
	std::shared_ptr<GateManager> m_gateMgr;

	GateKind m_kind;

	int m_effHandle;

	bool m_isBreak;
};

