#include "pch.h"
#include "Player.h"

void Player::OnDamaged(GameObjectRef attacker, int damage)
{
	GameObject::OnDamaged(attacker, damage);
}

void Player::OnDead(GameObjectRef attacker)
{
	GameObject::OnDead(attacker);
}
