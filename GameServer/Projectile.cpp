#include "pch.h"
#include "Projectile.h"

Projectile::Projectile()
{
	SetObjectType(GameObjectType::PROJECTILE);
}

void Projectile::Update()
{
	GameObject::Update();
}
