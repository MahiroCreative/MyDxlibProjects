#pragma once
#include <memory>
#include <vector>
#include <list>
#include "ObjectTag.h"
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
			StageGimmick,//ステージギミック
			Boss,//ボス
			Static,		// 動かない（最高）
		};
	public:

		float GetAngle(Vec3 a, Vec3 b)
		{
			a.Normalize();
			b.Normalize();

			float cos = Dot(a, b);
			// コサインをクリッピング
			cos = max(-1.0f, min(1.0f, cos));

			float rad = acos(cos);

#ifdef _DEBUG
			DrawFormatString(0, 125, 0xffffff, "rad(%f),deg(%f)", rad, rad * 180 / DX_PI_F);
#endif

			float epsilon = 1e-6f;
			if (cos > 1.0f - epsilon) {
				return 0;  // ほぼ同じベクトルなら角度0
			}
			return rad;
		}

		// 3Dモデルの上方向ベクトルを取得する関数
		Vec3 GetUpDirection(int modelHandle)
		{
			// モデルのワールド行列を取得
			MATRIX matrix;
			matrix=MV1GetMatrix(modelHandle);

			// ワールド行列の第2列が上方向ベクトル
			Vec3 upVector = { matrix.m[1][0], matrix.m[1][1], matrix.m[1][2] };

			return upVector;
		}



		Collidable(Priority priority, ObjectTag tag);
		Collidable(std::shared_ptr<Collidable> col);
		virtual ~Collidable();

		virtual void Update() = 0;
		virtual void Draw()=0;

		// 衝突したとき
		virtual void OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag) {}
		virtual void OnCollideStay(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag) {}
		virtual void OnCollideExit(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag) {}
		virtual void OnTriggerEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag) {}
		virtual void OnTriggerStay(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag) {}
		virtual void OnTriggerExit(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag) {}

		/* Getter */	
		ObjectTag GetTag() const { return m_tag; }
		std::shared_ptr<ColliderBase> GetColData(int index) const { return m_colliders.at(index); }
		Priority GetPriority() const { return m_priority; }
		void SetObjectTag(ObjectTag tag) { m_tag = tag; }
		bool IsAntiGravity() { return m_isAntiGravity; }
		void SetUpVec(Vec3 vel) { m_upVec = vel; }
		Vec3 GetUpVec() { return m_upVec; }
		Vec3 GetFrontVec() { return m_frontVec; }
		// 当たり判定を無視（スルー）するタグの追加/削除
		void AddThroughTag(ObjectTag tag);
		void RemoveThroughTag(ObjectTag tag);
		std::shared_ptr<Rigidbody> PlanetOnlyGetRigid() { return m_rigid; };
		void SetReverceGravityVec(Vec3 gravityReverce) { m_upVec = gravityReverce; }

		// 当たり判定を無視（スルー）する対象かどうか
		bool IsThroughTarget(std::shared_ptr<Collidable>) const;
		virtual bool IsDestroy() { return m_isDestroyFlag; }

		Vec3 GetKnockBackVelocity() { return (m_rigid->GetVelocity())*-1; }

		std::shared_ptr<Rigidbody> GetRigidbody() const { return m_rigid; }
	protected:
		std::shared_ptr<ColliderBase> AddCollider(const ColliderBase::Kind& kind,const ColliderBase::ColideTag& tag);
		void RemoveCollider(std::shared_ptr<ColliderBase> col);

		void SetAntiGravity(bool flag = true) { m_isAntiGravity=flag; }
	protected:
		// 物理データ
		std::shared_ptr<Rigidbody> m_rigid;
		// 当たり判定データ
		std::vector<std::shared_ptr<ColliderBase>> m_colliders;
		Vec3 m_upVec;
		Vec3 m_postUpVec;
		Vec3 m_frontVec;
		Vec3 m_sideVec;

		bool m_isDestroyFlag;
	
	private:
		std::list<ObjectTag>	throughTags;

		ObjectTag m_tag;
		Priority m_priority;
		bool m_isAntiGravity;
		
	};
}

