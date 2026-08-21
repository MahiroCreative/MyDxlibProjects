#pragma once
#include "Item.h"

#include"ColliderSphere.h"
#include"Physics.h"
#include"Quaternion.h"

class Coin :
    public Item
{
public:
    Coin(Vec3 pos, bool antiGravity = false);
    virtual ~Coin();

    void Init();

    virtual void OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);

};

