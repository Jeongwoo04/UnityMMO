#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
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
    C_Move movePkt;

    PlayerRef player = gameSession->_myPlayer;
    if (player == nullptr)
        return false;

    RoomRef room = player->GetRoom();
    if (room == nullptr)
        return false;

    room->DoAsync(&Room::HandleMove, player, movePkt);

    return true;
}

bool Handle_C_Skill(PacketSessionRef& session, Protocol::C_Skill& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
    C_Skill skillPkt;

    PlayerRef player = gameSession->_myPlayer;
    if (player == nullptr)
        return false;

    RoomRef room = player->GetRoom();
    if (room == nullptr)
        return false;

    room->DoAsync(&Room::HandleSkill, player, skillPkt);

    return true;
}
