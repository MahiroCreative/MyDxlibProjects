#pragma once
#include "Enemy.h"
#include"ColliderSphere.h"

using namespace MyEngine;
class Kuribo : public Enemy
{
public:
	Kuribo(Vec3 pos, int moveNum);
	virtual ~Kuribo();

	void Init();
	void Update();
	void SetMatrix();
	void Draw();


	virtual void OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
	virtual void OnTriggerStay(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);

	//メンバ関数ポインタ
	using MoveState_t = void(Kuribo::*)();
	MoveState_t m_moveUpdate;

private:
	/// <summary>
	/// アイドル時
	/// </summary>
	void SearchUpdate();
	/// <summary>
	/// ジャンプ時
	/// </summary>
	void JumpUpdate();
	/// <summary>
	/// 敵発見時
	/// </summary>
	void ChaseUpdate();
	/// <summary>
	/// 初期位置に戻る時
	/// </summary>
	void ComebackUpdate();
	/// <summary>
	/// 死亡時
	/// </summary>
	void DeathUpdate();

	//アニメーションの進行
	//ループしたかどうかを返す
	bool UpdateAnim(int attachNo);
	//アニメーションの変更
	void ChangeAnim(int animIndex, int speed = 1.f);

private:
	Vec3 m_vec;
	Vec3 m_attackDir;
	Vec3 m_moveShaftPos;
	Vec3 m_normVec;
	Vec3 m_nowPlanetPos;
	Vec3 m_comebackPoint;
	Vec3 m_targetPoint;
	std::shared_ptr<Collidable> m_player;
	std::shared_ptr<ColliderSphere> m_bodyCol;
	std::shared_ptr<ColliderSphere> m_searchCol;


	int m_chaseFrameCount;
	int m_initTime;

	int m_animationSpeed;

	//0.0f:prevが再生
	//1.0:currentが再生
	int m_currentAnimNo;//現在のアニメーション
	int m_prevAnimNo;//変更前のアニメーション
	float m_animBlendRate;//アニメーションの合成割合
	
};

