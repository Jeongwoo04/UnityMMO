#pragma once
#include "pch.h"
#include "Protocol.pb.h"
#include "JobQueue.h"
#include "Player.h"
#include "Monster.h"
#include "Projectile.h"
#include "Arrow.h"

class Room : public JobQueue
{
public:
    Room();

    int32 GetRoomId() { return _roomId; }
    void SetRoomId(int32 id) { _roomId = id; }

    MapRef GetMap() { return _map; }
    void SetMap(MapRef map) { _map = map; }

    void Init(int mapId);
    void Update();

    void EnterGame(GameObjectRef gameObject);
    void LeaveGame(int32 objectId);
    void Broadcast(SendBufferRef sendBuffer);

    void HandleMove(PlayerRef player, C_Move movePkt);
    void HandleSkill(PlayerRef player, C_Skill skillPkt);

    PlayerRef FindPlayer(const function<bool(GameObjectRef)>& condition);

    void ReserveRemoveObjects(int32 objectId);

    void RemoveObjects(int32 id);

public:
	USE_LOCK;
    int32 _roomId;

    std::unordered_map<int32, PlayerRef> _players;
    std::unordered_map<int32, MonsterRef> _monsters;
    std::unordered_map<int32, ProjectileRef> _projectiles;
    std::vector<int32> _removePendingObjects;

    MapRef _map;

	//unordered_map<uint64, int32>	_lastSentMessageIdPerUser;
	//Vector<GameSessionRef>	_sessions;
//
//private:
//	uint64 _nextCleanupTime = 0;
//	uint64 _nextPingTime = 0;
//	uint64 _nextPingCheckTime = 0;
};
