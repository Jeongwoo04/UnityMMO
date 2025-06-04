#pragma once
#include "GameObject.h"
#include "Enum.pb.h"
#include "Protocol.pb.h"

using PlayerRef = std::shared_ptr<class Player>;

class Player : public GameObject
{
public:
    Player()
    {
        SetObjectType(Protocol::GameObjectType::PLAYER);
    };

    void SetSession(GameSessionRef session) { _ownerSession = session; }
    GameSessionRef GetSession() { return _ownerSession.lock(); }

    virtual void OnDamaged(GameObjectRef attacker, int damage);
    virtual void OnDead(GameObjectRef attacker);

public:
    weak_ptr<GameSession> _ownerSession;
};