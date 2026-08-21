#pragma once
#include "Vector2.h"

struct Transform2D
{
	Vector2 Position{0,0};
	Vector2 Scale{1,1};
	float Rotation = 0.0f;
};
