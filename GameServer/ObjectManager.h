#pragma once
#include "GameObject.h"
#include "Player.h"
#include "Enum.pb.h"

using namespace Protocol;

class ObjectManager
{
public:
    static ObjectManager& Instance()
    {
        static ObjectManager instance;
        return instance;
    }

    static GameObjectType GetObjectTypeById(int id)
    {
        int type = (id >> 24) & 0x7F;
        return static_cast<GameObjectType>(type);
    }

    int GenerateId(GameObjectType type);

    template<typename T>
    shared_ptr<T> Add()
    {
        shared_ptr<T> gameObject = make_shared<T>();
        {
            WRITE_LOCK;

            gameObject->SetId(GenerateId(gameObject->GetObjectType()));

            if constexpr (std::is_same_v<T, Player>)
            {
                _players[gameObject->GetId()] = static_pointer_cast<Player>(gameObject);
            }
        }
        return gameObject;
    }

    bool Remove(int objectId);

    PlayerRef Find(int objectId);

private:
    ObjectManager() = default;

private:
    USE_LOCK;
    unordered_map<int, PlayerRef> _players;
    int _counter = 0;
};