#pragma once
#include <memory>
#include<list>

class Colidable;
class Enemy;

class EnemyManager
{
public:
	EnemyManager();

	virtual ~EnemyManager();
private:
	std::list<std::shared_ptr<Enemy>> m_enemies;
};

