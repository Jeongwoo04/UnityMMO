#pragma once
#include "Player.h"

using MonsterRef = std::shared_ptr<class Monster>;

class Monster : public GameObject
{
public:
    Monster();

    static GameObjectType GetStaticType() { return GameObjectType::MONSTER; }

    // FSM (Finite State Machine)
    virtual void Update() override;

    virtual void UpdateIdle();
    virtual void UpdateMoving();
    virtual void UpdateSkill();
    virtual void UpdateDead() { }

    virtual void OnDead(GameObjectRef attacker) override;

    void BroadcastMove();

    void SetPlayer(const PlayerRef& player) { _target = player; }
    PlayerRef GetPlayer() const { return _target.lock(); }
    
public:
    weak_ptr<Player> _target;

    int32 _searchCellDist = 10;
    int32 _chaseCellDist = 20;
    int32 _skillRange = 1;
    
    uint64 _nextSearchTick = 0;
    uint64 _nextMoveTick = 0;
    uint64 _coolTick = 0;
};