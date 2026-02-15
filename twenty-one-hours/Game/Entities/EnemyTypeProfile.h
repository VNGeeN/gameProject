#pragma once

#include <memory>

enum class EnemyType
{
    Grunt,
    Raider,
    Boss
};

struct EnemyStats
{
    int maxHealth = 60;
    int health = 60;
    int maxArmor = 15;
    int armor = 15;
    int damage = 8;
    float attackRange = 1.4f;
};

class EnemyTypeProfile
{
public:
    virtual ~EnemyTypeProfile() = default;

    virtual EnemyType getType() const = 0;
    virtual EnemyStats makeStats() const = 0;
    virtual float getSpeed() const = 0;
    virtual float getDetectionDistance() const = 0;
    virtual float getChaseDistance() const = 0;
    virtual float getSpriteWorldHeight() const = 0;

    bool isBoss() const { return getType() == EnemyType::Boss; }
};

std::shared_ptr<const EnemyTypeProfile> createEnemyTypeProfile(EnemyType type);