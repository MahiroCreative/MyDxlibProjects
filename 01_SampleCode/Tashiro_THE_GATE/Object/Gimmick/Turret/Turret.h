#pragma once
#include "Object/Object3DBase.h"
#include <list>
#include <memory>

class AnimController;
class Player;
class TurretBullet;

class Turret : public Object3DBase
{
public:
	Turret();
	~Turret();

	void Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity) override;
	void Init(const Vec3& dir, Player* player);
	void Update() override;
	void Draw() const override;

private:
	void LookUpdate();
	void AttackUpdate();

	void OnAttack();

private:
	std::shared_ptr<AnimController> m_anim;

	Player* m_player;

	std::list<std::shared_ptr<TurretBullet>> m_bulletList;

	Vec3 m_baseDir;
	Vec3 m_lookDir;

	int m_attackFrame;

	bool m_isAttack;
};

