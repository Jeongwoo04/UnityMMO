#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"
#include "Player.h"

GameSessionRef GameSessionManager::Generate()
{
	WRITE_LOCK;
	int32 sessionId = ++_sessionId;

	auto session = make_shared<GameSession>();
	session->SetSessionId(sessionId);
	_sessions[sessionId] = session;

	return session;
}

void GameSessionManager::Remove(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.erase(session->GetSessionId());
}

GameSessionRef GameSessionManager::Find(int32 sessionId)
{
	WRITE_LOCK;
	auto it = _sessions.find(sessionId);
	if (it == _sessions.end())
		return nullptr;

	GameSessionRef session = it->second;
	return session;
}


// 채팅 프로그램에서 전체 메시지
//void GameSessionManager::Broadcast(SendBufferRef sendBuffer) // for 돌면서 동일한 데이터를 보내주겠다. (복사비용 1번)
//{
//	WRITE_LOCK;
//	for (auto session : _sessions)
//	{
//		session.second->Send(sendBuffer); // -> loop 탈때 _sessions를 건드리는지 조심 !
//	}
//}