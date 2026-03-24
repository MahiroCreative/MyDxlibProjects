#pragma once
#include <memory>
#include <list>
#include <functional>
#include"Vec3.h"
#include "ObjectTag.h"
#include <DxLib.h>
#include"ColliderBase.h"



namespace MyEngine
{
	class Galaxy;
	class Rigidbody;
	class Collidable;

	class Physics final
	{
	private:
		friend Galaxy;

		enum class OnCollideInfoKind
		{
			CollideEnter,
			CollideStay,
			CollideExit,
			TriggerEnter,
			TriggerStay,
			TriggerExit
		};
		struct ColideInfo
		{
			std::shared_ptr<Collidable> col;
			ColliderBase::ColideTag tag;
		};
		struct OnCollideInfoData
		{
			std::shared_ptr<ColideInfo> own;
			std::shared_ptr<ColideInfo> send;
			OnCollideInfoKind kind;
		};
		
		struct CollideHitInfo
		{
			bool isHit = false;
			Vec3 hitPos;
		};
		using SendCollideInfo = std::unordered_map<std::shared_ptr<ColideInfo>, std::list<std::shared_ptr<ColideInfo>>>;
	
	private:

		Physics();

		Physics(const Physics&) = delete;
		void operator= (const Physics&) = delete;

	public:

		static Physics& GetInstance();

		void Entry(const std::shared_ptr<Collidable>& collidable);
		void Exit(const std::shared_ptr<Collidable>& collidable);

		void Update();
		void Draw();

		void Clear();

	private:

		void Gravity();
		void MoveNextPos() const;

		void CheckCollide();

		CollideHitInfo IsCollide(const std::shared_ptr<Rigidbody> rigidA, const std::shared_ptr<Rigidbody> rigidB, const std::shared_ptr<ColliderBase>& colliderA, const std::shared_ptr<ColliderBase>& colliderB) const;
		void FixNextPos(const std::shared_ptr<Rigidbody> primaryRigid, std::shared_ptr<Rigidbody> secondaryRigid, const std::shared_ptr<ColliderBase>& primaryCollider, const std::shared_ptr<ColliderBase>& secondaryCollider,const CollideHitInfo info);
		void AddNewCollideInfo(std::shared_ptr<ColideInfo> objA, std::shared_ptr<ColideInfo> objB, SendCollideInfo& info);
		void CheckSendOnCollideInfo(SendCollideInfo& preSendInfo, SendCollideInfo& newSendInfo, bool isTrigger);
		void AddOnCollideInfo(std::shared_ptr<ColideInfo> own, std::shared_ptr<ColideInfo> send, OnCollideInfoKind kind);
		void OnCollideInfo(std::shared_ptr<ColideInfo> own, std::shared_ptr<ColideInfo> send, OnCollideInfoKind kind);
		void FixPos() const;

	private:
		std::list<std::shared_ptr<Collidable>> m_collidables;
		std::list<std::shared_ptr<Collidable>> m_stageCollidables;

		std::list<OnCollideInfoData> m_onCollideInfo;
		SendCollideInfo m_newCollideInfo;
		SendCollideInfo m_preCollideInfo;
		SendCollideInfo m_newTirrigerInfo;
		SendCollideInfo m_preTirrigerInfo;
	};
}

