#pragma once
#include "Object/Object3DBase.h"

class Player;

class HandObject : public Object3DBase
{
private:
	// MEMO: もしfriendを使わずに済むならそちらの処理に変更する
	friend Player;

public:
	HandObject();
	~HandObject();

	void Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity) override;
	void Restart() override;

private:
	void OnHnad();
	void EndHand();

	void SetPos(const Vec3& pos);
	void ZeroGravity();

private:
	Vec3 m_startPos;
};

