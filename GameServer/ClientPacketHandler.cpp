#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Player.h"
#include "Room.h"
#include <ctime>
#include "FileUtils.h"
#include "DBConnectionPool.h"
#include "DBBind.h"
#include "XmlParser.h"
#include "DBSynchronizer.h"
#include "GenProcedures.h"
#include "GlobalQueue.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : log
	return false;
}

bool Handle_C_Move(PacketSessionRef& session, Protocol::C_Move& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

    PlayerRef player = gameSession->_myPlayer;
    if (player == nullptr)
        return false;

    RoomRef room = player->GetRoom();
    if (room == nullptr)
        return false;

    room->DoAsync(&Room::HandleMove, player, pkt);

    return true;
}

bool Handle_C_Skill(PacketSessionRef& session, Protocol::C_Skill& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

    PlayerRef player = gameSession->_myPlayer;
    if (player == nullptr)
        return false;

    RoomRef room = player->GetRoom();
    if (room == nullptr)
        return false;

    room->DoAsync(&Room::HandleSkill, player, pkt);

    return true;
}
