#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <algorithm>
#include "../Map/Map.h"
#include "EnemyTypeProfile.h"

class Player;
class RayCalc;
class Pseudo3DRenderer;

enum class EnemyState
{
    IDLE,   
    PATROL, 
    CHASE,  
    ATTACK, 
    DEAD    
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

enum class FacingDirection
{
    FORWARD,
    BACK,
    LEFT,
    RIGHT
};

class Enemy
{
public:
    Enemy(Map &map, float x, float y, EnemyType type = EnemyType::Grunt);

    bool isVisible(const sf::Vector2f &playerPos, const RayCalc &rayCalc) const;

    void update(sf::Time deltaTime, const sf::Vector2f &playerPos, float playerAngle);
    void render2D(sf::RenderTarget &target) const;
    void render3D(class Pseudo3DRenderer &renderer, const sf::Vector2f &playerPos, const RayCalc &rayCalc) const;

    bool isAlive() const { return mAlive; }
    void die() { mAlive = false; }
    void takeDamage(int amount);
    float getHitFlashIntensity() const { return std::max(0.0f, mHitFlashTimer / 0.18f); }

    sf::Vector2f getPosition() const { return mPosition; }
    void setPosition(const sf::Vector2f &position) { mPosition = position; }
    float getDetectionDistance() const { return mDetectionDistance; }

    sf::Time mAttackCooldown;
    sf::Time mAttackTimer;

    void attackPlayer(Player &player, sf::Time deltaTime);

    void setAnimationState(AnimationState state);
    AnimationState getAnimationState() const { return mAnimationState; }
    const EnemyStats &getStats() const { return mStats; }

    EnemyType getType() const { return mProfile->getType(); }
    bool isBoss() const { return mProfile->isBoss(); }

private:
    float mVisibility = 1.0f;
    float mHitFlashTimer = 0.0f;
    bool canSeePlayer(const sf::Vector2f &playerPos) const;
    void moveToPlayer(const sf::Vector2f &playerPos, float deltaTime);

    Map &mMap;
    sf::Vector2f mPosition;
    bool mAlive = true;
    EnemyStats mStats;
    std::shared_ptr<const EnemyTypeProfile> mProfile;
    float mSpriteWorldHeight = 1.0f;

    float mRadius = 0.25f;
    float mSpeed = 0.8f;
    float mDetectionDistance = 12.0f; 
    float mChaseDistance = 8.0f;      

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

    void applyTypeStats();
};