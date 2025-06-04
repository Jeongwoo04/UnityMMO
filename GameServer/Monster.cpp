#include "pch.h"
#include "Monster.h"
#include "GameObject.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "DataManager.h"
#include "RoomManager.h"
#include "Room.h"

Monster::Monster()
{
    SetObjectType(GameObjectType::MONSTER);

    // TEMP
    _statInfo()->set_level(1);
    _statInfo()->set_hp(100);
    _statInfo()->set_maxhp(100);
    _statInfo()->set_speed(5.0f);

    _posInfo()->set_state(CreatureState::IDLE);
}

void Monster::Update()
{
    switch (_posInfo()->state())
    {
    case CreatureState::IDLE:
        UpdateIdle();
        break;
    case CreatureState::MOVING:
        UpdateMoving();
        break;
    case CreatureState::SKILL:
        UpdateSkill();
        break;
    case CreatureState::DEAD:
        UpdateDead();
        break;
    }
}

void Monster::UpdateIdle()
{
    const uint64 tick = GetTickCount64();
    if (_nextSearchTick > tick)
        return;
    _nextSearchTick = tick + 1000;

    PlayerRef target = GetRoom()->FindPlayer([&](const GameObjectRef& p)
        {
            Vector2Int dir = p->GetCellPos() - GetCellPos();
            return dir.cellDist() <= _searchCellDist;
        });

    if (target == nullptr)
        return;

    SetPlayer(target);
    _posInfo()->set_state(CreatureState::MOVING);
}

void Monster::UpdateMoving()
{
    const uint64 tick = GetTickCount64();
    if (_nextMoveTick > tick)
        return;

    const int32 moveTick = static_cast<int32>(1000 / _statInfo()->speed());
    _nextMoveTick = tick + moveTick;

    PlayerRef target = GetPlayer();
    if (target == nullptr || target->GetRoom() != GetRoom())
    {
        SetPlayer(nullptr);
        _posInfo()->set_state(CreatureState::IDLE);
        BroadcastMove();
        return;
    }

    Vector2Int dir = target->GetCellPos() - GetCellPos();
    int32 dist = dir.cellDist();
    if (dist == 0 || dist > _chaseCellDist)
    {
        SetPlayer(nullptr);
        _posInfo()->set_state(CreatureState::IDLE);
        BroadcastMove();
        return;
    }

    vector<Vector2Int> path = GetRoom()->GetMap()->FindPath(GetCellPos(), target->GetCellPos(), /*checkObjects=*/false);
    if (path.size() < 2 || path.size() > _chaseCellDist)
    {
        SetPlayer(nullptr);
        _posInfo()->set_state(CreatureState::IDLE);
        BroadcastMove();
        return;
    }

    // Skill 가능 여부 체크
    if (dist <= _skillRange && (dir._x == 0 || dir._y == 0))
    {
        _coolTick = 0;
        _posInfo()->set_state(CreatureState::SKILL);
        return;
    }

    // 이동
    _posInfo()->set_movedir(GetDirFromVec(path[1] - GetCellPos()));
    GetRoom()->GetMap()->ApplyMove(shared_from_this(), path[1]);

    // 다른 플레이어에 알림
    BroadcastMove();
}

void Monster::BroadcastMove()
{
    S_Move movePkt;
    movePkt.set_objectid(GetId());
    *movePkt.mutable_posinfo() = *_posInfo();
    
    if (auto room = GetRoom())
    {
        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
        room->DoAsync(&Room::Broadcast, sendBuffer);
    }
}

void Monster::UpdateSkill()
{
    const uint64 tick = GetTickCount64();
    if (_coolTick == 0)
    {
        // 유효한 타겟 체크
        PlayerRef target = GetPlayer();
        if (target == nullptr || target->GetRoom() != GetRoom() || target->_statInfo()->hp() == 0)
        {
            SetPlayer(nullptr);
            _posInfo()->set_state(CreatureState::IDLE);
            BroadcastMove();
            return;
        }

        // 스킬 사용 가능한지
        Vector2Int dir = (target->GetCellPos() - GetCellPos());
        int32 dist = dir.cellDist();
        bool canUseSkill = (dist <= _skillRange && (dir._x == 0 || dir._y == 0));
        if (canUseSkill == false)
        {
            SetPlayer(nullptr);
            _posInfo()->set_state(CreatureState::IDLE);
            BroadcastMove();
            return;
        }

        // 타겟 방향 주시
        MoveDir lookDir = GetDirFromVec(dir);
        if (_posInfo()->movedir() != lookDir)
        {
            _posInfo()->set_movedir(lookDir);
            BroadcastMove();
        }

        SkillRef skillData = nullptr;
        auto it = DataManager::Instance().SkillDict.find(1);
        if (it == DataManager::Instance().SkillDict.end())
            return;
        skillData = it->second;

        // 데미지 판정
        target->OnDamaged(shared_from_this(), skillData->damage + _statInfo()->attack());

        // 스킬 사용 Broadcast
        S_Skill skillPkt;
        skillPkt.set_objectid(GetId());
        skillPkt.mutable_info()->set_skillid(skillData->id);
        
        if (auto room = GetRoom())
        {
            auto sendBuffer = ClientPacketHandler::MakeSendBuffer(skillPkt);
            room->DoAsync(&Room::Broadcast, sendBuffer);
        }

        // 스킬 쿨타임 적용
        _coolTick = tick + static_cast<int64>(1000 * skillData->cooldown);
    }

    if (_coolTick > tick)
        return;

    _coolTick = 0;
}

void Monster::OnDead(GameObjectRef attacker)
{
    auto room = GetRoom();
    if (room == nullptr)
        return;

    room->DoAsync(&Room::LeaveGame, GetId());

    _statInfo()->set_hp(_statInfo()->maxhp());
    _posInfo()->set_state(CreatureState::IDLE);
    _posInfo()->set_movedir(MoveDir::DOWN);
    _posInfo()->set_posx(5);
    _posInfo()->set_posy(5);

    room->DoAsync(&Room::EnterGame, shared_from_this());

    S_Die diePkt;
    diePkt.set_objectid(GetId());
    diePkt.set_attackerid(attacker->GetId());

    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(diePkt);
    room->DoAsync(&Room::Broadcast, sendBuffer);
}