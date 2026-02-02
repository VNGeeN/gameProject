#pragma once
#include <SFML/Graphics.hpp>
#include "../Map/Map.h"

class Player;
class RayCalc;
class Pseudo3DRenderer;

enum class EnemyState
{
    IDLE,   // Ожидание
    PATROL, // Патрулирование
    CHASE,  // Преследование
    ATTACK, // Атака
    DEAD    // Мертв
};

enum class AnimationState
{
    IDLE,
    WALK_RIGHT,
    WALK_LEFT,
    WALK_BACK,
    WALK_FORWARD,
    ATTACK,
    DEATH
};

// направление взгляда
enum class FacingDirection
{
    FORWARD, // Смотрит направо
    BACK,    // Смотрит наверх
    LEFT,    // Смотрит налево
    RIGHT    // Смотрит вниз
};

class Enemy
{
public:
    struct Stats
    {
        int maxHealth = 60;
        int health = 60;
        int maxArmor = 15;
        int armor = 15;
        int damage = 8;
        float attackRange = 1.4f;
    };

    Enemy(Map &map, float x, float y);

    bool isVisible(const sf::Vector2f &playerPos, const RayCalc &rayCalc) const;

    void update(sf::Time deltaTime, const sf::Vector2f &playerPos, float playerAngle);
    void render2D(sf::RenderTarget &target) const;
    void render3D(class Pseudo3DRenderer &renderer, const sf::Vector2f &playerPos, const RayCalc &rayCalc) const;

    bool isAlive() const { return mAlive; }
    void die() { mAlive = false; }
    void takeDamage(int amount);

    sf::Vector2f getPosition() const { return mPosition; }
    void setPosition(const sf::Vector2f &position) { mPosition = position; }
    float getDetectionDistance() const { return mDetectionDistance; }

    sf::Time mAttackCooldown;
    sf::Time mAttackTimer;

    void attackPlayer(Player &player, sf::Time deltaTime);

    void setAnimationState(AnimationState state);
    AnimationState getAnimationState() const { return mAnimationState; }
    const Stats &getStats() const { return mStats; }

private:
    float mVisibility = 1.0f;
    bool canSeePlayer(const sf::Vector2f &playerPos) const;
    void moveToPlayer(const sf::Vector2f &playerPos, float deltaTime);

    Map &mMap;
    sf::Vector2f mPosition;
    bool mAlive = true;
    Stats mStats;

    // Параметры ИИ
    float mRadius = 0.25f;
    float mSpeed = 0.8f;
    float mDetectionDistance = 12.0f; // видит на 12 клеток
    float mChaseDistance = 8.0f;      // преследует до 8 клеток

    // LOD
    mutable int mFrameCounter = 0;

    EnemyState mState = EnemyState::PATROL;
    sf::Vector2f mPatrolTarget;
    sf::Time mStateTimer;

    void updateState(sf::Time deltaTime, const sf::Vector2f &playerPos);
    void patrol(sf::Time deltaTime);

    // Анимация и таймеры - ТОЛЬКО ОДИН РАЗ
    AnimationState mAnimationState = AnimationState::IDLE;
    int mCurrentFrame = 0;
    sf::Clock mFrameTimer;
    float mFrameRate = 0.1f; // 10 кадров/секунду

    sf::Vector2f mLastPosition;
    FacingDirection mFacingDirection = FacingDirection::FORWARD;
};