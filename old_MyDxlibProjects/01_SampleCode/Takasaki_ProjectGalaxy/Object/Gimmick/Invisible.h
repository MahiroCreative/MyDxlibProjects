#pragma once
#include "Collidable.h"
using namespace MyEngine;
class Invisible : public Collidable
{
public:
	Invisible(Vec3 pos);
	virtual ~Invisible();

	void Init();
	void Update();
	void Draw();

private:

};

