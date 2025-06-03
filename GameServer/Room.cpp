#include "pch.h"
#include "Room.h"
#include "Map.h"
#include "GameSession.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "ClientPacketHandler.h"
#include "DataManager.h"

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

Room::Room()
{
    _map = make_shared<Map>();
}

void Room::Init(int mapId)
{
	_map->LoadMap(mapId);

	// TEMP
	MonsterRef monster = ObjectManager::Instance().Add<Monster>();
	monster->SetCellPos(Vector2Int(5, 5));
	DoAsync(&Room::EnterGame, static_pointer_cast<GameObject>(monster));
}

void Room::Update()
{
	for (auto& [id, m] : _monsters)
	{
		m->Update();
	}
	for (auto& [id, p] : _projectiles)
	{
		p->Update();
	}
}

void Room::EnterGame(GameObjectRef gameObject)
{
    if (gameObject == nullptr)
        return;

    GameObjectType type = ObjectManager::GetObjectTypeById(gameObject->GetId());

    if (type == GameObjectType::PLAYER)
    {
        PlayerRef player = static_pointer_cast<Player>(gameObject);
        _players[player->GetId()] = player;
        player->SetRoom(static_pointer_cast<Room>(shared_from_this()));

        _map->ApplyMove(player, Vector2Int(player->_posInfo()->posx(), player->_posInfo()->posy()));

        // 나에게 정보 전송
        {
            S_EnterGame enterPkt;
            *enterPkt.mutable_player() = player->_info;
            {
                auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterPkt);
                player->GetSession()->Send(sendBuffer);
            }

            S_Spawn spawnPkt;
            for (auto& [id, p] : _players)
            {
                if (player != p)
                    *spawnPkt.add_objects() = p->_info;
            }

            for (auto& [id, mon] : _monsters)
                *spawnPkt.add_objects() = mon->_info;

            for (auto& [id, pro] : _monsters)
                *spawnPkt.add_objects() = pro->_info;

            {
                auto sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
                player->GetSession()->Send(sendBuffer);
            }
        }
    }
    else if (type == GameObjectType::MONSTER)
    {
        MonsterRef monster = static_pointer_cast<Monster>(gameObject);
        _monsters[gameObject->GetId()] = monster;
        monster->SetRoom(static_pointer_cast<Room>(shared_from_this()));
        _map->ApplyMove(monster, Vector2Int(monster->_posInfo()->posx(), monster->_posInfo()->posy()));
    }
    else if (type == GameObjectType::PROJECTILE)
    {
        ProjectileRef projectile = static_pointer_cast<Projectile>(gameObject);
        _projectiles[gameObject->GetId()] = projectile;
        projectile->SetRoom(static_pointer_cast<Room>(shared_from_this()));
    }

    // 타인에 정보 전송
    {
        S_Spawn spawnPkt;
        *spawnPkt.add_objects() = gameObject->_info;
        for (auto& [id, p] : _players)
        {
            if (id != gameObject->GetId())
            {
                auto sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
                p->GetSession()->Send(sendBuffer);
            }
        }
    }
}

void Room::LeaveGame(int32 objectId)
{
    GameObjectType type = ObjectManager::GetObjectTypeById(objectId);
    //GetObjectTypeById(objectId);

    if (type == GameObjectType::PLAYER)
    {
        auto it = _players.find(objectId);
        if (it == _players.end())
            return;
        PlayerRef player = it->second;
        player->SetRoom(nullptr);
        _players.erase(objectId);

        _map->ApplyLeave(player);

        // 나에게 정보 전송
        {
            S_LeaveGame leavePkt;
            auto sendBuffer = ClientPacketHandler::MakeSendBuffer(leavePkt);
            player->GetSession()->Send(sendBuffer);
        }
    }
    else if (type == GameObjectType::MONSTER)
    {
        auto it = _monsters.find(objectId);
        if (it == _monsters.end())
            return;
        MonsterRef monster = it->second;
        monster->SetRoom(nullptr);
        _monsters.erase(objectId);

        _map->ApplyLeave(monster);
    }
    else if (type == GameObjectType::PROJECTILE)
    {
        auto it = _projectiles.find(objectId);
        if (it == _projectiles.end())
            return;

        ProjectileRef projectile = it->second;
        projectile->SetRoom(nullptr);
        _projectiles.erase(objectId);
    }

    // 타인에 정보 전송
    {
        S_Despawn despawnPkt;
        despawnPkt.add_objectids(objectId);
        for (auto& [id, p] : _players)
        {
            if (id != objectId)
            {
                auto sendBuffer = ClientPacketHandler::MakeSendBuffer(despawnPkt);
                p->GetSession()->Send(sendBuffer);
            }
        }
    }
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
    for (auto& [i, p] : _players)
    {
        p->GetSession()->Send(sendBuffer);
    }
}

void Room::HandleMove(PlayerRef player, C_Move movePkt)
{
    if (player == nullptr)
        return;
    // TODO 검증

    // 서버에서 좌표 이동
    PositionInfo destPosInfo = movePkt.posinfo();
    ObjectInfo info = player->_info;

    // 다른 좌표로 이동할 경우, 체크
    if (destPosInfo.posx() != info.posinfo().posx() || destPosInfo.posy() != info.posinfo().posy())
    {
        if (_map->CanGo(Vector2Int(destPosInfo.posx(), destPosInfo.posy())) == false)
            return;
    }

    player->_posInfo()->CopyFrom(movePkt.posinfo());
    _map->ApplyMove(player, Vector2Int(destPosInfo.posx(), destPosInfo.posy()));

    // 다른 플레이어에 알려주기
    S_Move resMovePkt;
    resMovePkt.set_objectid(player->GetId());
    *resMovePkt.mutable_posinfo() = movePkt.posinfo();

    {
        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(resMovePkt);
        this->DoAsync(&Room::Broadcast, sendBuffer);
    }
}

void Room::HandleSkill(PlayerRef player, C_Skill skillPkt)
{
    if (player == nullptr)
        return;

    if (player->_posInfo()->state() != CreatureState::IDLE)
        return;

    // TODO : 스킬 사용 가능여부 체크

    player->_posInfo()->set_state(CreatureState::SKILL);
    S_Skill skill;
    skill.set_objectid(player->GetId());
    skill.mutable_info()->set_skillid(2);

    {
        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(skill);
        Broadcast(sendBuffer);
    }

    SkillRef skillData = nullptr;
    auto it = DataManager::Instance().SkillDict.find(skillPkt.info().skillid());
    if (it == DataManager::Instance().SkillDict.end())
        return;
    skillData = it->second;

    switch (skillData->skillType)
    {
    case SkillType::SKILL_AUTO:
    {
        Vector2Int skillPos = player->GetFrontCellPos(player->_posInfo()->movedir());
        GameObjectRef target = _map->Find(skillPos);
        if (target != nullptr)
        {
            //Console.WriteLine("Hit GameObject !");
        }
    }
    break;
    case SkillType::SKILL_PROJECTILE:
    {
        ArrowRef arrow = ObjectManager::Instance().Add<Arrow>();
        if (arrow == nullptr)
            return;

        arrow->SetOwner(player);
        arrow->SetData(skillData);

        arrow->_posInfo()->set_state(CreatureState::MOVING);
        arrow->_posInfo()->set_movedir(player->_posInfo()->movedir());
        arrow->_posInfo()->set_posx(player->_posInfo()->posx());
        arrow->_posInfo()->set_posy(player->_posInfo()->posy());
        arrow->_statInfo()->set_speed(skillData->projectile->speed);
        this->DoAsync(&Room::EnterGame, static_pointer_cast<GameObject>(arrow));
    }
    break;
    }
}

PlayerRef Room::FindPlayer(const function<bool(GameObjectRef)>& condition)
{
    for (auto& [i, p] : _players)
    {
        if (condition(p))
            return p;
    }

    return nullptr;
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
