#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "Protocol.pb.h" // StatInfo, SkillType 등이 정의되어 있어야 함

using json = nlohmann::json;
using namespace Protocol;

// ProjectileInfo 구조체
struct ProjectileInfo
{
    std::string name;
    float speed = 0;
    int range = 0;
    std::string prefab;
};

// Skill 구조체
struct Skill
{
    int id = 0;
    std::string name;
    float cooldown = 0;
    int damage = 0;
    SkillType skillType = SkillType::SKILL_AUTO;
    std::shared_ptr<ProjectileInfo> projectile; // optional
};

SkillType ToSkillType(const std::string& str);

// 템플릿 인터페이스
template<typename Key, typename Value>
class ILoader
{
public:
    virtual std::unordered_map<Key, std::shared_ptr<Value>> MakeDict() const = 0;
    virtual ~ILoader() = default;
};

// StatData
class StatData : public ILoader<int, StatInfo>
{
public:
    std::vector<std::shared_ptr<StatInfo>> stats;

    std::unordered_map<int, std::shared_ptr<StatInfo>> MakeDict() const override;

    static std::shared_ptr<StatData> LoadFromJson(const std::string& path);
};

// SkillData
class SkillData : public ILoader<int, Skill>
{
public:
    std::vector<std::shared_ptr<Skill>> skills;

    std::unordered_map<int, std::shared_ptr<Skill>> MakeDict() const override;

    static std::shared_ptr<SkillData> LoadFromJsonFile(const std::string& path); // 이름 다르게!
};