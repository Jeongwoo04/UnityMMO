#include "pch.h"
#include "ObjectManager.h"

int ObjectManager::GenerateId(GameObjectType type)
{
    return (static_cast<int>(type) << 24) | (_counter++);
}

bool ObjectManager::Remove(int objectId)
{
    GameObjectType type = GetObjectTypeById(objectId);

    WRITE_LOCK;
    if (type == GameObjectType::PLAYER)
    {
        return _players.erase(objectId) > 0;
    }

    return false;
}

PlayerRef ObjectManager::Find(int objectId)
{
    GameObjectType type = GetObjectTypeById(objectId);

    READ_LOCK;
    if (type == GameObjectType::PLAYER)
    {
        auto it = _players.find(objectId);
        if (it != _players.end())
            return it->second;
    }

    return nullptr;
}