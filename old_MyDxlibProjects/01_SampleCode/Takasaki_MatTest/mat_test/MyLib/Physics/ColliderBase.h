#pragma once
#include <unordered_map>

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

		// コンストラクタ
		ColliderBase(Kind kind);
		virtual ~ColliderBase() {}

		// 当たり判定種別取得
		Kind GetKind() const { return m_kind; }
		// 当たった情報更新
		void UpdateHit(const ColliderBase* collider, bool isHit);
		bool IsHit(const ColliderBase* collider) const { return m_isHit.at(collider); }
		bool IsPreHit(const ColliderBase* collider) const { return m_isPreHit.at(collider); }


	public:
		// MEMO: 色々なところで使うためColliderBase関係のpublic変数は
		// メンバ変数のコーディング規約無視している
		bool isTrigger;

	private:

		Kind m_kind;
		std::unordered_map<const ColliderBase*, bool> m_isHit;
		std::unordered_map<const ColliderBase*, bool> m_isPreHit;
	};
}
