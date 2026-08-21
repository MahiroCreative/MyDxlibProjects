#include "WatchGalaxy.h"
#include"Camera.h"
#include"DumbBellPlanet.h"
#include"Pad.h"


using namespace std;

WatchGalaxy::WatchGalaxy(std::shared_ptr<Player> playerPointer):Galaxy(playerPointer)
{
	player = playerPointer;
	planet.push_back(make_shared<DumbbellPlanet>(Vec3(0, 0, 0), 0xffff00, -1));
	camera = make_shared<Camera>();
}

WatchGalaxy::~WatchGalaxy()
{
}

void WatchGalaxy::Init()
{
}

void WatchGalaxy::Update()
{
	if (Pad::IsPress(KEY_INPUT_UP))
	{
		cameraPos.z++;
	}
	if (Pad::IsPress(KEY_INPUT_DOWN))
	{
		cameraPos.z--;
	}
	if (Pad::IsPress(KEY_INPUT_RIGHT))
	{
		cameraPos.x++;
	}
	if (Pad::IsPress(KEY_INPUT_LEFT))
	{
		cameraPos.x--;
	}
	if (Pad::IsPress(KEY_INPUT_1))
	{
		cameraPos.y++;
	}
	if (Pad::IsPress(KEY_INPUT_2))
	{
		cameraPos.y--;
	}
	camera->SetCameraPoint(cameraPos);
}

void WatchGalaxy::Draw()
{
}
