//#pragma once
//
//class Vec2
//{
//public:
//	float x;
//	float y;
//	Vec2() : x(0), y(0) {}
//	Vec2(float inx, float iny) : x(inx), y(iny) {}
//
//	void operator+=(const Vec2& in) {
//		x += in.x;
//		y += in.y;
//	}
//	void operator*=(float scale) {
//		x *= scale;
//		y *= scale;
//	}
//	void operator-=(const Vec2& in) {
//		x -= in.x;
//		y -= in.y;
//	}
//	Vec2 operator+(const Vec2& rv) {
//		return Vec2(x + rv.x, y + rv.y);
//	}
//
//	Vec2 operator-(const Vec2& rv) {
//		return Vec2(x - rv.x, y - rv.y);
//	}
//
//	Vec2 operator*(const float scale) {
//		return Vec2(x * scale, y * scale);
//	}
//
//	
//	float Length()const;
//	
//	Vec2 Normalized()const;
//	
//};
