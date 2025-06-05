#pragma once

class RoomManager
{
public:
    static RoomManager& Instance()
    {
        static RoomManager instance;
        return instance;
    }

    RoomRef Add(int mapId);
    bool Remove(int roomId);
    RoomRef Find(int roomId);

    void UpdateAllRooms();

public:
    USE_LOCK;
    unordered_map<int32, RoomRef> _rooms;
    int32 _roomId = 1;
};