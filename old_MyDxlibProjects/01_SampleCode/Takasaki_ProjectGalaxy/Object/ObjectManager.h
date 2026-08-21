#pragma once
#include<list>
#include"Collidable.h"
#include"Vec3.h"

using namespace MyEngine;
class ObjectManager
{
public:
	ObjectManager();
	virtual ~ObjectManager();

	void Update();
	void Draw();

private:
	std::list<std::shared_ptr<Collidable>> objects;
	Vec3 cursorPos;

};

