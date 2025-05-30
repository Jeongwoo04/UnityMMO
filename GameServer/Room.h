#pragma once
#include "JobQueue.h"

using PlayerRef = shared_ptr<class Player>;

class Room : public JobQueue
{
public:
	void Enter(GameSessionRef gameSession, PlayerRef player);
	void Leave(GameSessionRef gameSession, PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);
	
public:
	USE_LOCK;
	unordered_map<uint64, PlayerRef>	_players;
	//unordered_map<uint64, int32>	_lastSentMessageIdPerUser;
	//Vector<GameSessionRef>	_sessions;

private:
	uint64 _nextCleanupTime = 0;
	uint64 _nextPingTime = 0;
	uint64 _nextPingCheckTime = 0;
};
