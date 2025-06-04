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
    _updateScheduled.store(false);

	for (auto& [id, m] : _monsters)
	{
		m->Update();
	}
	for (auto& [id, p] : _projectiles)
	{
		p->Update();
	}
}

void Room::ScheduleUpdate()
{
    bool expected = false;
    if (_updateScheduled.compare_exchange_strong(expected, true))
    {
        DoAsync(&Room::Update);
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

            for (auto& [id, pro] : _projectiles)
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

        _players.erase(player->GetId());
        _map->ApplyLeave(player);
        player->SetRoom(nullptr);

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

        _monsters.erase(objectId);
        _map->ApplyLeave(monster);
        monster->SetRoom(nullptr);
    }
    else if (type == GameObjectType::PROJECTILE)
    {
        auto it = _projectiles.find(objectId);
        if (it == _projectiles.end())
            return;

        ProjectileRef projectile = it->second;

        _projectiles.erase(objectId);
        projectile->SetRoom(nullptr);
    }

    // 타인에 정보 전송
    {
        S_Despawn despawnPkt;
        despawnPkt.add_objectids(objectId);
        for (auto& [id, p] : _players)
        {
            if (p->GetId() != objectId)
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

#define LOG(level) std::wcout << "[" << #level << "] "

void Room::HandleMove(PlayerRef player, C_Move movePkt)
{
    if (player == nullptr)
        return;
    // TODO 검증

    // 서버에서 좌표 이동
    PositionInfo destPosInfo = movePkt.posinfo();
    ObjectInfo info = player->_info;

    LOG(INFO) << "[MOVE] PlayerId: " << player->GetId()
        << " MoveDir: " << destPosInfo.movedir()
        << " From (" << info.posinfo().posx() << "," << info.posinfo().posy() << ")"
        << " To (" << destPosInfo.posx() << "," << destPosInfo.posy() << ")" << endl;

    if (destPosInfo.posx() != info.posinfo().posx() || destPosInfo.posy() != info.posinfo().posy())
    {
        if (_map->CanGo(Vector2Int(destPosInfo.posx(), destPosInfo.posy())) == false)
        {
            LOG(WARNING) << "[MOVE] PlayerId: " << player->GetId()
                << " tried to move to invalid cell: ("
                << destPosInfo.posx() << "," << destPosInfo.posy() << ")" << endl;
            return;
        }
    }

    // 다른 좌표로 이동할 경우, 체크
    //if (destPosInfo.posx() != info.posinfo().posx() || destPosInfo.posy() != info.posinfo().posy())
    //{
    //    if (_map->CanGo(Vector2Int(destPosInfo.posx(), destPosInfo.posy())) == false)
    //        return;
    //}

    player->_posInfo()->set_state(destPosInfo.state());
    player->_posInfo()->set_movedir(destPosInfo.movedir());
    _map->ApplyMove(player, Vector2Int(destPosInfo.posx(), destPosInfo.posy()));

    // 다른 플레이어에 알려주기
    S_Move resMovePkt;
    resMovePkt.set_objectid(player->_info.objectid());
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

    if (player->_posInfo()->state() != CreatureState::IDLE) {
        LOG(WARNING) << "[SKILL] PlayerId: " << player->GetId()
            << " tried to cast skill while not idle." << endl;
        return;
    }
    // TODO : 스킬 사용 가능여부 체크
    const auto& skillInfo = skillPkt.info();
    LOG(INFO) << "[SKILL] PlayerId: " << player->GetId()
        << " used SkillId: " << skillInfo.skillid() << endl;

    player->_posInfo()->set_state(CreatureState::SKILL);
    S_Skill skill;
    skill.set_objectid(player->_info.objectid());
    skill.mutable_info()->set_skillid(2);

    {
        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(skill);
        Broadcast(sendBuffer);
    }

    SkillRef skillData = nullptr;
    auto it = DataManager::Instance().SkillDict.find(skillPkt.info().skillid());
    if (it == DataManager::Instance().SkillDict.end()) {
        LOG(ERROR) << "[SKILL] PlayerId: " << player->GetId()
            << " used unknown SkillId: " << skillPkt.info().skillid() << endl;
        return;
    }
    skillData = it->second;

    switch (skillData->skillType)
    {
    case SkillType::SKILL_AUTO:
    {
        Vector2Int skillPos = player->GetFrontCellPos(player->_posInfo()->movedir());
        GameObjectRef target = _map->Find(skillPos);

        LOG(INFO) << "[SKILL_AUTO] PlayerId: " << player->GetId()
            << " SkillPos: (" << skillPos._x << "," << skillPos._y << ")"
            << " Target: " << (target ? target->GetId() : 0) << endl;
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
        {
            LOG(ERROR) << "[SKILL_PROJECTILE] PlayerId: " << player->GetId()
                << " failed to create Arrow" << endl;
            return;
        }

        LOG(INFO) << "[SKILL_PROJECTILE] PlayerId: " << player->GetId()
            << " FireArrow from (" << player->_posInfo()->posx() << "," << player->_posInfo()->posy() << ")"
            << " Dir: " << player->_posInfo()->movedir() << endl;

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
