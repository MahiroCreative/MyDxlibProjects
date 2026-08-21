#pragma once
#include <memory>
#include <list>
#include <vector>
#include "ObjectTag.h"
#include "Utility.h"
#include "Vec3.h"
#include "Collider/ColliderBase.h"

namespace MyEngine
{
	class Rigidbody;
	class Collidable;

	class Physics final
	{
	private:
		enum class OnCollideInfoKind
		{
			CollideEnter,
			CollideStay,
			CollideExit,
			TriggerEnter,
			TriggerStay,
			TriggerExit
		};
		struct SendInfo
		{
			std::weak_ptr<Collidable> own;
			std::weak_ptr<Collidable> send;
			int ownColIndex;
			int sendColIndex;
			CollideHitInfo hitInfo;
		};
		struct OnCollideInfoData
		{
			std::weak_ptr<Collidable> own;
			std::weak_ptr<Collidable> send;
			int ownColIndex;
			int sendColIndex;
			CollideHitInfo hitInfo;
			OnCollideInfoKind kind;
		};
		struct HitTagObjectInfo
		{
			Collidable* col;
			CollideHitInfo hitInfo;
		};
		using SendCollideInfo = std::list<SendInfo>;
	private:
		Physics();

		Physics(const Physics&) = delete;
		void operator= (const Physics&) = delete;
	public:
		static Physics& GetInstance();

		/// <summary>
		/// Physicsに登録する
		/// </summary>
		/// <param name="collidable">登録する情報</param>
		void Entry(std::shared_ptr<Collidable> collidable);
		/// <summary>
		/// Physicsから削除する
		/// </summary>
		/// <param name="collidable">削除する情報</param>
		void Exit(std::shared_ptr<Collidable> collidable);

		/// <summary>
		/// 更新処理
		/// </summary>
		void Update();

		/// <summary>
		/// 指定したタグと当たっているオブジェクトを取得する
		/// </summary>
		/// <param name="center">判定する座標の中心</param>
		/// <param name="collider">判定する当たり判定データ</param>
		/// <param name="tag">判定したいオブジェクトのタグ</param>
		/// <param name="preInfo">一つ前の当たった情報</param>
		/// <param name="isOne">true:一つでも当たっていることを知れればよい /false: 全ての当たっていることを知る</param>
		/// <param name="through">無視するオブジェクト(nullptrだと全て見る)</param>
		/// <returns>当たっている情報テーブル</returns>
		std::vector<HitTagObjectInfo> GetHitObject(const Vec3& center, ColliderBase* collider, ObjectTag tag, const PreHitInfo& preInfo = PreHitInfo{}, bool isOne = false, Collidable* through = nullptr);

	private:
		/// <summary>
		/// オブジェクトが持つ移動量から次の移動座標を決定する
		/// </summary>
		void MoveNextPos() const;

		/// <summary>
		/// 判定確認
		/// </summary>
		void CheckCollide();
		/// <summary>
		/// 判定リストの取得
		/// </summary>
		/// <returns>判定リスト</returns>
		std::vector<std::shared_ptr<Collidable>> GetCollisionList() const;
		/// <summary>
		/// 判定通知リストに追加する
		/// </summary>
		/// <param name="objA">オブジェクトA</param>
		/// <param name="objB">オブジェクトB</param>
		/// <param name="info">通知リスト</param>
		/// <param name="hitInfo">当たった情報</param>
		void AddNewCollideInfo(const std::weak_ptr<Collidable>& objA, const std::weak_ptr<Collidable>& objB, int colIndexA, int colIndexB, SendCollideInfo& info, const CollideHitInfo& hitInfo);
		/// <summary>
		/// 通知を種類(当たった時・当たっている状態・離れた時)に分ける
		/// </summary>
		/// <param name="preSendInfo">一つ前の送った通知リスト</param>
		/// <param name="newSendInfo">今回送る通知リスト</param>
		/// <param name="isTrigger">true:OnCollide～に送る /false: OnTrigger～に送る</param>
		void CheckSendOnCollideInfo(SendCollideInfo& preSendInfo, SendCollideInfo& newSendInfo, bool isTrigger);
		/// <summary>
		/// 最終的な判定通知リストに追加する
		/// </summary>
		/// <param name="info">情報</param>
		/// <param name="kind">通知の種類</param>
		void AddOnCollideInfo(const SendInfo& info, OnCollideInfoKind kind);
		/// <summary>
		/// 通知の種類に合わせて通知関数を呼ぶ
		/// </summary>
		/// <param name="own">送る側</param>
		/// <param name="send">通知を受け取る側</param>
		/// <param name="ownIndex">判定データの要素番号(自身)</param>
		/// <param name="sendIndex">判定データの要素番号(対象)</param>
		/// <param name="hitInfo">当たった情報</param>
		/// <param name="kind">通知の種類</param>
		void OnCollideInfo(const std::weak_ptr<Collidable>& own, const std::weak_ptr<Collidable>& send, int ownIndex, int sendIndex, const CollideHitInfo& hitInfo, OnCollideInfoKind kind);
		/// <summary>
		/// 座標を確定させる
		/// </summary>
		void FixPos() const;

#ifdef _DEBUG
		/// <summary>
		/// デバック情報の追加
		/// </summary>
		/// <param name="pos">座標</param>
		/// <param name="colliders">当たり判定でーった</param>
		/// <param name="color">色</param>
		void AddDebugInfo(const Vec3& pos, const std::vector<std::shared_ptr<ColliderBase>>& colliders, unsigned int color) const;
#endif

	private:
		std::vector<std::shared_ptr<Collidable>> m_objects;
		std::list<OnCollideInfoData> m_onCollideInfo;
		SendCollideInfo m_newCollideInfo;
		SendCollideInfo m_preCollideInfo;
		SendCollideInfo m_newTirrigerInfo;
		SendCollideInfo m_preTirrigerInfo;
	};
}

