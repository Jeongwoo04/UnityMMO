#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif

#include "CorePch.h"
#include "Enum.pb.h"
#include <functional>

using GameSessionRef = std::shared_ptr<class GameSession>;
using GameObjectRef = std::shared_ptr<class GameObject>;
using PlayerRef = std::shared_ptr<class Player>;
using MonsterRef = std::shared_ptr<class Monster>;
using ProjectileRef = std::shared_ptr<class Projectile>;
using ArrowRef = std::shared_ptr<class Arrow>;
using RoomRef = std::shared_ptr<class Room>;
using MapRef = std::shared_ptr<class Map>;
using SkillRef = std::shared_ptr<class Skill>;
using StatRef = std::shared_ptr<class Stat>;