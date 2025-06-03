#pragma once

#include "Session.h"

class GameSession : public PacketSession // sealed로 인해 OnRecv 사용불가
{
public:
	GameSession()
	{

	}

	~GameSession()
	{
		wcout << "~GameSession : SessionId = " << _sessionId << endl;
	}

	virtual void	OnConnected() override;
	virtual void	OnDisconnected() override;
	virtual void	OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void	OnSend(int32 len) override;

public:
	int32 GetSessionId() const { return _sessionId; }
	void SetSessionId(int32 sessionId) { _sessionId = sessionId; }
	//uint64 GetLastPongTime() { return _lastPongTime; }

public:
	PlayerRef	_myPlayer;
	int32		_sessionId;
	//uint64 _lastPongTime;
};