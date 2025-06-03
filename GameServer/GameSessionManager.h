#pragma once

class GameSession;

using GameSessionRef = shared_ptr<GameSession>;

class GameSessionManager
{
public:
	static GameSessionManager& Instance()
	{
		static GameSessionManager instance;
		return instance;
	}

	GameSessionRef Generate();
	void Remove(GameSessionRef session);
	GameSessionRef Find(int32 sessionId);
	
private:
	GameSessionManager() = default;
	USE_LOCK;
	int32 _sessionId = 0;
	unordered_map<int32, GameSessionRef> _sessions;
};
