#include "pch.h"
#include "GameObject.h"
#include "ClientPacketHandler.h"
#include "Room.h"

const Vector2Int Vector2Int::Up = Vector2Int(0, 1);
const Vector2Int Vector2Int::Down = Vector2Int(0, -1);
const Vector2Int Vector2Int::Left = Vector2Int(-1, 0);
const Vector2Int Vector2Int::Right = Vector2Int(1, 0);

void GameObject::Update()
{

}

void GameObject::Init(const ObjectInfo& info)
{
	_info = info;
}

Vector2Int GameObject::GetCellPos()
{
	return Vector2Int(_posInfo()->posx(), _posInfo()->posy());
}

void GameObject::SetCellPos(const Vector2Int& pos)
{
	_posInfo()->set_posx(pos._x);
	_posInfo()->set_posy(pos._y);
}

Vector2Int GameObject::GetFrontCellPos()
{
	return GetFrontCellPos(_posInfo()->movedir());
}

Vector2Int GameObject::GetFrontCellPos(Protocol::MoveDir dir)
{
	Vector2Int cellPos = GetCellPos();

	switch (dir)
	{
	case Protocol::MoveDir::UP:
		cellPos += Vector2Int::Up;
		break;
	case Protocol::MoveDir::DOWN:
		cellPos += Vector2Int::Down;
		break;
	case Protocol::MoveDir::LEFT:
		cellPos += Vector2Int::Left;
		break;
	case Protocol::MoveDir::RIGHT:
		cellPos += Vector2Int::Right;
		break;
	}

	return cellPos;
}

MoveDir GameObject::GetDirFromVec(const Vector2Int& dir)
{
	if (dir._x > 0)
		return MoveDir::RIGHT;
	else if (dir._x < 0)
		return MoveDir::LEFT;
	else if (dir._y > 0)
		return MoveDir::UP;
	else
		return MoveDir::DOWN;
}

void GameObject::OnDamaged(GameObjectRef attacker, int damage)
{
	auto room = _room.lock();
	if (!room)
		return;

	SetHp(GetHp() - damage);

	S_ChangeHp changeHpPkt;
	changeHpPkt.set_objectid(GetId());
	changeHpPkt.set_hp(GetHp());

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(changeHpPkt);
	room->DoAsync(&Room::Broadcast, sendBuffer);

	if (GetHp() <= 0)
	{
		OnDead(attacker);
	}
}

void GameObject::OnDead(GameObjectRef attacker)
{
	auto room = _room.lock();
	if (!room)
		return;

	room->DoAsync(&Room::LeaveGame, GetId());

	SetHp(_statInfo()->maxhp());
	_posInfo()->set_state(CreatureState::IDLE);
	_posInfo()->set_movedir(MoveDir::DOWN);
	_posInfo()->set_posx(0);
	_posInfo()->set_posy(0);

	room->DoAsync(&Room::EnterGame, shared_from_this());

	S_Die diePkt;
	diePkt.set_objectid(GetId());
	diePkt.set_attackerid(attacker->GetId());

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(diePkt);
	room->DoAsync(&Room::Broadcast, sendBuffer);
}
