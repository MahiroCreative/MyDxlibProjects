//#include "Geometry.h"
//#include <algorithm>
//Rect::Rect() :center(0, 0),size() {}
//
//Rect::Rect(float x, float y, float w, float h) : center(x, y), size(w, h) {}
//
//Rect::Rect(Vec2& pos, Size& sz) : center(pos), size(sz) {}
//
//const float
//Rect::Left()const {
//	return center.x - size.w / 2;
//}
//const float
//Rect::Top()const {
//	return center.y - size.h / 2;
//}
//const float
//Rect::Right()const {
//	return center.x + size.w / 2;
//}
//const float
//Rect::Bottom()const {
//	return center.y + size.h / 2;
//}
//
//void Rect::Draw(unsigned int color) {
//	DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
//	DxLib::DrawBox(static_cast<int>(Left()), static_cast<int>(Top()), static_cast<int>(Right()), static_cast<int>(Bottom()), color, true);
//	DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
//	DxLib::DrawBox(static_cast<int>(Left()), static_cast<int>(Top()), static_cast<int>(Right()), static_cast<int>(Bottom()), color, false);
//}
////
////void Rect::Draw(const Vec2& offset, unsigned int color) {
////	DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
////	DxLib::DrawBox(static_cast<int>(Left() + offset.x),
////		static_cast<int>(Top() + offset.y),
////		static_cast<int>(Right() + offset.x),
////		static_cast<int>(Bottom() + offset.y),
////		color, true);
////	DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
////	DxLib::DrawBox(static_cast<int>(Left() + offset.x),
////		static_cast<int>(Top() + offset.y),
////		static_cast<int>(Right() + offset.x),
////		static_cast<int>(Bottom() + offset.y),
////		color, false);
////}
//
//void Rect::Draw(const Vec2& offset, unsigned int color) {
//	DxLib::DrawBox(static_cast<int>(Left() + offset.x),
//		static_cast<int>(Top() + offset.y),
//		static_cast<int>(Right() + offset.x),
//		static_cast<int>(Bottom() + offset.y),
//		color, false);
//}
//
//
//Rect Rect::CreateRectFromLRTB(float left, float right, float top, float bottom) {
//	return Rect((left + right) / 2, (top + bottom) / 2, right - left, bottom - top);
//}
//
//Rect Rect::CreateOverlappedRangeRect(const Rect& rcA, const Rect& rcB) {
//	return CreateRectFromLRTB(std::max(rcA.Left(), rcB.Left()), std::min(rcA.Right(), rcB.Right()), std::max(rcA.Top(), rcB.Top()), std::min(rcA.Bottom(), rcB.Bottom()));
//}
