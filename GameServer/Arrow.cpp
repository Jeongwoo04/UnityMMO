#include "pch.h"
#include "Arrow.h"
#include "GameObject.h"
#include "ClientPacketHandler.h"
#include "DataManager.h"
#include "Room.h"

void Arrow::Update()
{
    if (GetData() == nullptr || GetData()->projectile == nullptr || GetOwner() == nullptr || GetOwner()->GetRoom() == nullptr)
        return;

    const uint64 tick = GetTickCount64();
    if (_nextMoveTick > tick)
        return;

    const uint64 moveTick = static_cast<uint64>(1000 / GetData()->projectile->speed);
    _nextMoveTick = tick + moveTick;

    Vector2Int destPos = GetFrontCellPos();
    auto room = GetRoom();
    if (room == nullptr)
        return;
    if (room->GetMap()->CanGo(destPos))
    {
        SetCellPos(destPos);

        S_Move movePkt;
        movePkt.set_objectid(GetId());
        *movePkt.mutable_posinfo() = *_posInfo();

        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
        room->Broadcast(sendBuffer);

        //Console.WriteLine("Move Arrow");
    }
    else
    {
        GameObjectRef target = room->GetMap()->Find(destPos);
        if (target != nullptr && target != _owner.lock())
        {
            target->OnDamaged(shared_from_this(), GetOwner()->_statInfo()->attack() + GetData()->damage);

        }
            // ¼Ò¸ê
        room->LeaveGame(GetId());
    }
}
