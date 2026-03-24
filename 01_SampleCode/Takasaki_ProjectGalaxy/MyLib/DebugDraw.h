#pragma once
#include <list>
#include "Vec3.h"

namespace MyEngine
{
	class DebugDraw
	{
	public:
		struct CircleInfo
		{
			Vec3 center = Vec3();
			float radius = 0.0f;
			unsigned int color = 0xff00ff;
		};
		struct LineInfo
		{
			Vec3 start = Vec3();
			Vec3 end = Vec3();
			unsigned int color = 0xff00ff;
		};
		struct SphereInfo
		{
			Vec3 center = Vec3();
			float radius = 0.0f;
			unsigned int color = 0xff00ff;
			int dirNum = 8;
			bool isFill = false;
		};
	private:
		DebugDraw();
		~DebugDraw();

		DebugDraw(const DebugDraw&) = delete;
		void operator= (const DebugDraw&) = delete;

	public:
		static DebugDraw& GetInstance();

		void Gulid() const;

		void Clear();
		void Draw() const;

		void DrawCircle(const CircleInfo& circleInfo);
		void DrawLine(const LineInfo& lineInfo);
		void DrawSphere(const SphereInfo& sphereInfo);

	public:
		static constexpr unsigned int COL_BEFOR  = 0x0000ff;
		static constexpr unsigned int COL_NEXT   = 0xff00ff;
		static constexpr unsigned int COL_AFFTER = 0xff0000;

	private:
		std::list<CircleInfo> m_circleInfo;
		std::list<LineInfo> m_lineInfo;
		std::list<SphereInfo> m_sphereInfo;
	};
}

