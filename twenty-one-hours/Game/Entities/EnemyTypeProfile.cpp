#include "EnemyTypeProfile.h"

namespace
{
class GruntProfile final : public EnemyTypeProfile
{
public:
    EnemyType getType() const override { return EnemyType::Grunt; }
    EnemyStats makeStats() const override { return EnemyStats{}; }
    float getSpeed() const override { return 0.8f; }
    float getDetectionDistance() const override { return 12.0f; }
    float getChaseDistance() const override { return 8.0f; }
    float getSpriteWorldHeight() const override { return 1.0f; }
};

class RaiderProfile final : public EnemyTypeProfile
{
public:
    EnemyType getType() const override { return EnemyType::Raider; }
    EnemyStats makeStats() const override
    {
        return EnemyStats{45, 45, 5, 5, 6, 1.2f};
    }
    float getSpeed() const override { return 1.2f; }
    float getDetectionDistance() const override { return 14.0f; }
    float getChaseDistance() const override { return 10.0f; }
    float getSpriteWorldHeight() const override { return 0.9f; }
};

class BossProfile final : public EnemyTypeProfile
{
public:
    EnemyType getType() const override { return EnemyType::Boss; }
    EnemyStats makeStats() const override
    {
        return EnemyStats{220, 220, 50, 50, 18, 2.0f};
    }
    float getSpeed() const override { return 0.6f; }
    float getDetectionDistance() const override { return 16.0f; }
    float getChaseDistance() const override { return 12.0f; }
    float getSpriteWorldHeight() const override { return 1.15f; }
};
} // namespace

std::shared_ptr<const EnemyTypeProfile> createEnemyTypeProfile(EnemyType type)
{
    static const auto grunt = std::make_shared<const GruntProfile>();
    static const auto raider = std::make_shared<const RaiderProfile>();
    static const auto boss = std::make_shared<const BossProfile>();

    switch (type)
    {
    case EnemyType::Grunt:
        return grunt;
    case EnemyType::Raider:
        return raider;
    case EnemyType::Boss:
        return boss;
    }

    return grunt;
}