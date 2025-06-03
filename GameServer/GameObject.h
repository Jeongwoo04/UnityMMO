#pragma once
#include "pch.h"
#include "Map.h"
#include "Enum.pb.h"
#include "Protocol.pb.h"

using namespace Protocol;
using RoomRef = std::shared_ptr<class Room>;
using GameObjectRef = std::shared_ptr<class GameObject>;

class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
    virtual void Update();

    int32 GetId() const { return _info.objectid(); }
    void SetId(const int32& id) { _info.set_objectid(id); }

    int32 GetHp() const { return _info.statinfo().hp(); }
    void SetHp(const int32& hp) { _info.mutable_statinfo()->set_hp(hp); }

    RoomRef GetRoom() const { return _room.lock(); }
    void SetRoom(const RoomRef& room) { _room = room; }

    GameObjectType GetObjectType() { return _objectType; }
    void SetObjectType(const Protocol::GameObjectType& type) { _objectType = type; }

    Vector2Int GetCellPos();
    void SetCellPos(const Vector2Int& pos);

    Vector2Int GetFrontCellPos();
    Vector2Int GetFrontCellPos(Protocol::MoveDir dir);

    MoveDir GetDirFromVec(const Vector2Int& dir);

    virtual void OnDamaged(GameObjectRef attacker, int damage);
    virtual void OnDead(GameObjectRef attacker);

    PositionInfo* _posInfo() { return _info.mutable_posinfo(); }
    StatInfo* _statInfo() { return _info.mutable_statinfo(); }

    void Init(const ObjectInfo& info);

public:
    ObjectInfo _info;
    GameObjectType _objectType = GameObjectType::NONE;
    weak_ptr<Room> _room;
};