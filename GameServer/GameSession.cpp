#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "ObjectManager.h"
#include "RoomManager.h"
#include "DataManager.h"
#include "Room.h"

void GameSession::OnConnected()
{
	GameSessionManager::Instance().Generate();
	//_lastPongTime = ::GetTickCount64();

	_myPlayer = ObjectManager::Instance().Add<Player>();
	{
		std::stringstream ss;
		ss << "Player_" << _myPlayer->_info.objectid();
		_myPlayer->_info.set_name(ss.str());
		_myPlayer->_posInfo()->set_state(CreatureState::IDLE);
		_myPlayer->_posInfo()->set_movedir(MoveDir::DOWN);
		_myPlayer->_posInfo()->set_posx(0);
		_myPlayer->_posInfo()->set_posy(0);

		auto it = DataManager::Instance().StatDict.find(1);
		if (it == DataManager::Instance().StatDict.end())
			return;

		_myPlayer->_statInfo()->MergeFrom(*it->second);
		_myPlayer->SetSession(static_pointer_cast<GameSession>(shared_from_this()));
	}

	RoomRef room = RoomManager::Instance().Find(1);
	_myPlayer->SetRoom(room);
	room->DoAsync(&Room::EnterGame, static_pointer_cast<GameObject>(_myPlayer));

	wcout << "OnConnected" << endl;
}

void GameSession::OnDisconnected()
{
	GameSessionManager::Instance().Remove(static_pointer_cast<GameSession>(shared_from_this()));
	
	RoomRef room = RoomManager::Instance().Find(1);
	room->DoAsync(&Room::LeaveGame, _myPlayer->GetId());

	_myPlayer = nullptr;
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO : packetId 대역 체크
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{

}