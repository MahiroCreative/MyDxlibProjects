#pragma once
#include "Vec3.h"

namespace MyEngine
{
	class Rigidbody final
	{
	public:
		Rigidbody();
		~Rigidbody();

		void Init(const Vec3& pos = Vec3());

		// 座標取得
		const Vec3& GetPos() const { return m_pos; }
		// 次の座標を取得
		const Vec3& GetNextPos() const { return m_nextPos; }
		// 移動ベクトルを取得
		const Vec3& GetVelocity() const { return m_velocity; }
		// 移動方向を取得
		const Vec3& GetDir() const { return m_dir; }
		// 重力を使用するか
		bool IsGravity() const { return m_isGravity; }

		// 場所の設定
		void SetPos(const Vec3& pos) { m_pos = pos; }
		// 次の座標の設定
		void SetNextPos(const Vec3& nextPos) { m_nextPos = nextPos; }
		// 移動ベクトルを設定
		void SetVelocity(const Vec3& velocity);
		// 重量の設定
		void SetGravity(bool isGravity) { m_isGravity = isGravity; }

		/// <summary>
		/// ある方向に力を加える
		/// </summary>
		/// <param name="dir">力の方向</param>
		/// <param name="power">力の強さ</param>
		/// <param name="isLimit">true:速度に限界値を設定する /false:限界値を設定しない</param>
		/// <param name="limit">限界値</param>
		/// <param name="isMaxUp">true:速度を限界値に修正する /false:修正しない</param>
		void AddForce(const Vec3& dir, float power, bool isLimit = false, float limit = 0.0f, bool isMaxUp = false);

	private:
		bool m_isGravity;
		Vec3 m_pos;
		Vec3 m_nextPos;
		Vec3 m_velocity;
		Vec3 m_dir;
	};
}

