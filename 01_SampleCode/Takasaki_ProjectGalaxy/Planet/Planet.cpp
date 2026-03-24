#include "Planet.h"
#include"MyLib/Physics/ColliderBase.h"


Planet::Planet():Collidable(Priority::Static, ObjectTag::Stage),
m_isSearch(false),
	gravityPower(0),
	modelH(-1)
{
	
}

Planet::Planet(int modelHandle):Collidable(Priority::Static,ObjectTag::Stage),
	gravityPower(0),
	modelH(modelHandle)
{
	
}

Planet::~Planet()
{
}

void Planet::Init()
{
}

void Planet::Update()
{
}

void Planet::Draw()
{
	
}