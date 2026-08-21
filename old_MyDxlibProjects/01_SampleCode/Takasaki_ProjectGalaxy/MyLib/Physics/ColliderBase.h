#pragma once
#include <unordered_map>
#include<memory>
#include"Vec3.h"

namespace MyEngine
{
	class ColliderBase abstract
	{
	public:
		// 当たり判定種別
		enum class Kind
		{
			Sphere,
			Capsule,
			Box,
		};


		enum class ColideTag
		{
			Head,
			Body,
			Fist,
			Foot,
			Spin,
			one,//汎用的なタグ1
			two,//汎用的なタグ2
			three//汎用的なタグ3
		};


		// コンストラクタ
		ColliderBase(Kind kind,ColideTag tag);
		virtual ~ColliderBase() {}

		// 当たり判定種別取得
		Kind GetKind() const { return m_kind; }
		// 当たった情報更新
		void UpdateHit(const ColliderBase* collider, bool isHit);
		bool IsHit(const ColliderBase* collider) const { return m_isHit.at(collider); }
		bool IsPreHit(const ColliderBase* collider) const { return m_isPreHit.at(collider); }
		bool NowOnHit() { return m_isNowOnHit; }
		void SetNowOnHit(bool flag) { m_isNowOnHit = flag; }
		Vec3 GetShift() const{ return m_posShift; }
		ColideTag GetTag()const { return m_tag; }
		/// <summary>
		/// オブジェクトの座標から当たり判定の相対位置をセット
		/// </summary>
		/// <param name="shift"></param>
		void SetShiftPosNum(Vec3 shift) { m_posShift = shift; }
	public:
		// MEMO: 色々なところで使うためColliderBase関係のpublic変数は
		// メンバ変数のコーディング規約無視している
		bool isTrigger;

	private:
		//オブジェクトの位置からの当たり判定の相対量
		Vec3 m_posShift;
		Kind m_kind;
		ColideTag m_tag;
		std::unordered_map<const ColliderBase*, bool> m_isHit;
		std::unordered_map<const ColliderBase*, bool> m_isPreHit;
		bool m_isNowOnHit;
	};
}
