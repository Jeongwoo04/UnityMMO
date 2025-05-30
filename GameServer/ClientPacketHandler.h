#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_S_EnterGame = 0,
	PKT_S_LeaveGame = 1,
	PKT_S_Spawn = 2,
	PKT_S_Despawn = 3,
	PKT_C_Move = 4,
	PKT_S_Move = 5,
	PKT_C_Skill = 6,
	PKT_S_Skill = 7,
	PKT_S_ChangeHp = 8,
	PKT_S_Die = 9,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_Move(PacketSessionRef& session, Protocol::C_Move& pkt);
bool Handle_C_Skill(PacketSessionRef& session, Protocol::C_Skill& pkt);

class ClientPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_Move] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_Move>(Handle_C_Move, session, buffer, len); };
		GPacketHandler[PKT_C_Skill] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_Skill>(Handle_C_Skill, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_EnterGame& pkt) { return MakeSendBuffer(pkt, PKT_S_EnterGame); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LeaveGame& pkt) { return MakeSendBuffer(pkt, PKT_S_LeaveGame); }
	static SendBufferRef MakeSendBuffer(Protocol::S_Spawn& pkt) { return MakeSendBuffer(pkt, PKT_S_Spawn); }
	static SendBufferRef MakeSendBuffer(Protocol::S_Despawn& pkt) { return MakeSendBuffer(pkt, PKT_S_Despawn); }
	static SendBufferRef MakeSendBuffer(Protocol::S_Move& pkt) { return MakeSendBuffer(pkt, PKT_S_Move); }
	static SendBufferRef MakeSendBuffer(Protocol::S_Skill& pkt) { return MakeSendBuffer(pkt, PKT_S_Skill); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ChangeHp& pkt) { return MakeSendBuffer(pkt, PKT_S_ChangeHp); }
	static SendBufferRef MakeSendBuffer(Protocol::S_Die& pkt) { return MakeSendBuffer(pkt, PKT_S_Die); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T & pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};