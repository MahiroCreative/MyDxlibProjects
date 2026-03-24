#pragma once
#include <memory>
#include <vector>
#include "../ObjectTag.h"
#include "Rigidbody.h"
#include "ColliderBase.h"

namespace MyEngine
{
	class Physics;
//	class ColliderBase;

	/// <summary>
	/// 衝突できるもの
	/// </summary>
	class Collidable : public std::enable_shared_from_this<Collidable>
	{
		friend Physics;
	public:
		// 優先度判別
		enum class Priority : int
		{
			Low,		// 低
			Middle,		// 中
			High,		// 高
			Static,		// 動かない（最高）
		};
	public:
		Collidable(Priority priority, ObjectTag tag);
		Collidable(std::shared_ptr<Collidable> col);
		virtual ~Collidable();

		virtual void Update() = 0;
		virtual void Draw()=0;

		// 衝突したとき
		virtual void OnCollideEnter(std::shared_ptr<Collidable> colider) {}
		virtual void OnCollideStay(std::shared_ptr<Collidable> colider) {}
		virtual void OnCollideExit(std::shared_ptr<Collidable> colider) {}
		virtual void OnTriggerEnter(std::shared_ptr<Collidable> colider) {}
		virtual void OnTriggerStay(std::shared_ptr<Collidable> colider) {}
		virtual void OnTriggerExit(std::shared_ptr<Collidable> colider) {}

		/* Getter */
		ObjectTag GetTag() const { return m_tag; }
		Priority GetPriority() const { return m_priority; }
		void SetObjectTag(ObjectTag tag) { m_tag = tag; }
		bool IsAntiGravity() { return m_isAntiGravity; }
		void SetUpVec(Vec3 vel) { m_upVec = vel; }
		// 当たり判定を無視（スルー）するタグの追加/削除
		void AddThroughTag(ObjectTag tag);
		void RemoveThroughTag(ObjectTag tag);
		std::shared_ptr<Rigidbody> PlanetOnlyGetRigid() { return m_rigid; };
		void SetReverceGravityVec(Vec3 gravityReverce) { m_upVec = gravityReverce; }

		// 当たり判定を無視（スルー）する対象かどうか
		bool IsThroughTarget(std::shared_ptr<Collidable>) const;

		Vec3 GetKnockBackVelocity() { return (m_rigid->GetVelocity())*-1; }

		std::shared_ptr<Rigidbody> GetRigidbody() const { return m_rigid; }
	protected:
		std::shared_ptr<ColliderBase> AddCollider(const ColliderBase::Kind& kind);

		void SetAntiGravity(bool flag = true) { m_isAntiGravity=flag; }
	protected:
		// 物理データ
		std::shared_ptr<Rigidbody> m_rigid;
		// 当たり判定データ
		std::list<std::shared_ptr<ColliderBase>> m_colliders;
		Vec3 m_upVec;

		
	
	private:
		std::list<ObjectTag>	throughTags;

		ObjectTag m_tag;
		Priority m_priority;
		bool m_isAntiGravity;
	};
}

