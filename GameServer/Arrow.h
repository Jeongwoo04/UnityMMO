#pragma once
#include "Projectile.h"
#include "GameObject.h"

class Arrow : public Projectile
{
public:
    void SetOwner(const GameObjectRef& gameObject) { _owner = gameObject; }
    GameObjectRef GetOwner() { return _owner.lock(); }

    virtual void Update() override;

public:
    weak_ptr<GameObject> _owner;
    uint64 _nextMoveTick = 0;
};