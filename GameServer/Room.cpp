#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"
#include "GlobalQueue.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include <Convert.h>

//void Room::Update()
//{
//	const uint64 now = ::GetTickCount64();
//
//	if (now >= _nextCleanupTime)
//	{
//		CleanupPlayers();                // 죽은 세션 정리
//		_nextCleanupTime = now + 1000;
//	}
//
//	if (now >= _nextPingCheckTime)
//	{
//		CheckPingTimeout();              // 응답 없는 세션 킥
//		_nextPingCheckTime = now + 5000;
//	}
//
//	if (now >= _nextPingTime)
//	{
//		BroadcastPing();                 // Ping 전송
//		_nextPingTime = now + 5000;
//	}
//}

void Room::Enter(GameSessionRef gameSession, PlayerRef player)
{
	
}

void Room::Leave(GameSessionRef gameSession, PlayerRef player)
{
	
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& [id, player] : _players)
	{
		auto session = player->ownerSession.lock();
		if (session)
		{
			session->Send(sendBuffer);
		}
	}
}

//void Room::CleanupPlayers()
//{
//	for (auto it = _players.begin(); it != _players.end(); )
//	{
//		const auto& player = it->second;
//		if (!player || player->ownerSession.expired())
//		{
//			it = _players.erase(it);
//		}
//		else
//		{
//			++it;
//		}
//	}
//}

//void Room::BroadcastPing()
//{
//	uint64 now = ::GetTickCount64();
//
//	Protocol::S_PING pingPkt;
//	pingPkt.set_timestamp(now);
//	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pingPkt);
//
//	wcout << "Server Send : Broadcast Ping test. Time = " << now << endl;
//
//	DoAsync(&Room::Broadcast, sendBuffer);
//}

//void Room::CheckPingTimeout()
//{
//	if (_players.empty())
//		return;
//
//	uint64 now = ::GetTickCount64();
//
//	// 순회 도중 Leave -> _players.erase 위험
//	for (auto& [id, player] : _players)
//	{
//		if (!player)
//			continue; // nullptr 보호
//
//		auto session = player->ownerSession.lock();
//		if (!session)
//			continue;
//
//		if ((now - session->_lastPongTime) >= 20000)
//		{
//			wcout << L"[Ping Timeout] Kicking session: " << session->GetSessionId() << endl;
//
//			// 곧바로 Disconnect X, Room Job으로 Kick(Disconnect) 예약
//			PlayerRef p = player;
//			DoAsync(&Room::Kick, p);
//		}
//	}
//}

//void Room::Kick(PlayerRef player)
//{
//	auto session = player->ownerSession.lock();
//	if (session)
//		session->Disconnect(L"Ping Timeout");
//}
