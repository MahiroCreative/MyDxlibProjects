#pragma once
#include "Enemy.h"
#include"ColliderSphere.h"
using namespace MyEngine;

class SpaceEmperor : public Enemy
{
public:
	SpaceEmperor(Vec3 pos);
	virtual ~SpaceEmperor();

	void Init();
	void Update();
	void SetMatrix();
	void Draw();

	void SetTarget(std::shared_ptr<Collidable> target) { m_target = target; }
	void OnBossPlanet();
	bool GetIsFind() { return m_isFindTarget; }
	Vec3 GetNeckPos() const{ return m_neckPos; }
	virtual void OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);

	//メンバ関数ポインタ
	using emperorState_t = void(SpaceEmperor::*)();
	emperorState_t m_update;
private:

	void DoNothingUpdate();
	void IntroUpdate();
	void IdleUpdate();
	void AttackUpdate();
	void HitUpdate();

	//アニメーションの進行
	//ループしたかどうかを返す
	bool UpdateAnim(int attachNo);
	//アニメーションの変更
	void ChangeAnim(int animIndex, int speed = 1.f);

	std::shared_ptr<MyEngine::ColliderSphere> m_armCol;

	bool m_isFindTarget;

	int m_currentAnimNo;//現在のアニメーション
	int m_prevAnimNo;//変更前のアニメーション
	float m_animBlendRate;//アニメーションの合成割合

	int m_neckFrameIndex;//モデルの首のフレームのインデックス
	float m_armExtensionSpeed;
	int m_armExtensionDistance;

	float m_animSpeed;
	Vec3 m_hitDir;
	Vec3 m_neckNowDir;
	Vec3 m_neckPos;
	Vec3 m_armMoveDir;
	std::shared_ptr<Collidable> m_target;
};

