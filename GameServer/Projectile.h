#pragma once
#include "pch.h"

using SkillRef = std::shared_ptr<class Skill>;
using ProjectileRef = std::shared_ptr<class Projectile>;

class Projectile : public GameObject
{
public:
    Projectile();
    
    void SetData(SkillRef skill) { _data = skill; }
    SkillRef GetData() const { return _data; }

    virtual void Update();

private:
    SkillRef _data;
};