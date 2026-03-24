//#pragma once
//#include"Vec2.h"
//#include"Vec3.h"
//
////サイズを表す構造体
//struct Size {
//public:
//	Size() :w(0), h(0) {}
//	Size(float inw, float inh) :w(inw), h(inh) {}
//	float w;
//	float h;
//};
//
/////矩形を表現する構造体
//class Rect {
//public:
//	Vec2 center;
//	Size size;
//	Rect();
//	Rect(float x, float y, float w,float h);
//	Rect(Vec2& pos, Size& sz);
//	const float Left()const;
//	const float Top()const;
//	const float Right() const;
//	const float Bottom()const;
//	const float Width()const { return size.w; }
//	const float Height()const { return size.h; }
//
//	///矩形を描画する
//	///@param color 色を0x00000000～0xffffffffで指定する(デフォルト白)
//	void Draw(unsigned int color = 0xffffffff);
//
//	///矩形を描画する(オフセットつき)
//	///@param offset ずらした位置に表示したい場合にオフセット値を指定
//	///@param color 色を0x00000000～0xffffffffで指定する(デフォルト白)
//	void Draw(const Vec2& offset, unsigned int color = 0xffffffff);
//	
//	///左、右、上、下から矩形を作る
//	///@param left 左
//	///@param right 右
//	///@param top 上
//	///@param bottom 下
//	static Rect CreateRectFromLRTB(float left, float right, float top, float bottom);
//
//	///二つの矩形から重なり矩形を作る
//	///@param rcA 矩形A
//	///@param rcB 矩形B
//	static Rect CreateOverlappedRangeRect(const Rect& rcA, const Rect& rcB);
//};
//
//class Geometry
//{
//};
//
