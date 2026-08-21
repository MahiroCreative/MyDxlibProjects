#include"Galaxy.h"



namespace
{

}

Galaxy::Galaxy(std::shared_ptr<Player> playerPointer) :
	m_isClearFlag(false),
	m_isGameOverFlag(false)
{
	player = playerPointer;
}

Galaxy::~Galaxy()
{
}
