#include "pch.h"
#include "Room.h"
#include "RoomManager.h"

RoomRef RoomManager::Add(int mapId)
{
    RoomRef room = make_shared<Room>();

    WRITE_LOCK;
    {
        room->SetRoomId(_roomId);
        _rooms[_roomId] = room;
        _roomId++;
    }

    room->DoAsync(&Room::Init, mapId);

    return room;
}

bool RoomManager::Remove(int roomId)
{
    WRITE_LOCK;
    return _rooms.erase(roomId) > 0;
}

RoomRef RoomManager::Find(int roomId)
{
    READ_LOCK;
    auto it = _rooms.find(roomId);
    if (it != _rooms.end())
        return it->second;

    return nullptr;
}

void RoomManager::UpdateAllRooms()
{
    for (auto& [id, room] : _rooms)
    {
        room->DoAsync(&Room::Update);
    }
}