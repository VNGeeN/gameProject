#include "Enemy.h"
#include "../Rendering/Render/Pseudo3DRenderer.h"
#include "../Ray/RayCalc.h"
#include <cmath>
#include <random>
#include <algorithm>
#include "../Player/Player.h"

Enemy::Enemy(Map &map, float x, float y, EnemyType type)
    : mMap(map), mPosition(x, y), mProfile(createEnemyTypeProfile(type)), mAttackCooldown(sf::seconds(1.0f)),
      mAttackTimer(sf::Time::Zero), mStateTimer(sf::Time::Zero)
{
    mStats = mProfile->makeStats();
    mSpeed = mProfile->getSpeed();
    mDetectionDistance = mProfile->getDetectionDistance();
    mChaseDistance = mProfile->getChaseDistance();
    mSpriteWorldHeight = mProfile->getSpriteWorldHeight();
}

bool Enemy::canSeePlayer(const sf::Vector2f &playerPos) const
{
    float dx = playerPos.x - mPosition.x;
    float dy = playerPos.y - mPosition.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > mDetectionDistance)
        return false;

    int steps = std::min(10, static_cast<int>(distance * 2));
    for (int i = 1; i < steps; i++)
    {
        float t = i / static_cast<float>(steps);
        float checkX = mPosition.x + dx * t;
        float checkY = mPosition.y + dy * t;

        if (mMap.isWall(checkX, checkY))
        {
            return false;
        }
    }
    return true;
}

void Enemy::moveToPlayer(const sf::Vector2f &playerPos, float deltaTime)
{
    float dx = playerPos.x - mPosition.x;
    float dy = playerPos.y - mPosition.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 0.5f)
    {
        dx /= distance;
        dy /= distance;
        float newX = mPosition.x + dx * mSpeed * deltaTime;
        float newY = mPosition.y + dy * mSpeed * deltaTime;

        if (!mMap.isWall(newX, newY))
        {
            float playerDistance = sqrt(
                (newX - playerPos.x) * (newX - playerPos.x) +
                (newY - playerPos.y) * (newY - playerPos.y));

            if (playerDistance > 0.8f)
            {
                mPosition.x = newX;
                mPosition.y = newY;
            }
        }
    }
}

void Enemy::update(sf::Time deltaTime, const sf::Vector2f &playerPos, float playerAngle)
{
    mLastPosition = mPosition;
    mHitFlashTimer = std::max(0.0f, mHitFlashTimer - deltaTime.asSeconds());
    if (!mAlive)
    {
        mAnimationState = AnimationState::DEATH;
        if (mFrameTimer.getElapsedTime().asSeconds() > mFrameRate)
        {
            mFrameTimer.restart();
            mCurrentFrame = (mCurrentFrame + 1) % 4;
        }
        return;
    }

    mFrameCounter++;
    float distanceToPlayer = std::sqrt(
        std::pow(playerPos.x - mPosition.x, 2) +
        std::pow(playerPos.y - mPosition.y, 2));

    if (distanceToPlayer > 15.0f && mFrameCounter % 3 != 0)
        return;

    float angleToEnemy = std::atan2(mPosition.y - playerPos.y, mPosition.x - playerPos.x);
    float angleDiff = std::abs(angleToEnemy - playerAngle);
    if (angleDiff > M_PI)
        angleDiff = 2 * M_PI - angleDiff;

    if (angleDiff < M_PI / 2.0f || distanceToPlayer < 8.0f)
    {
        if (canSeePlayer(playerPos))
        {
            moveToPlayer(playerPos, deltaTime.asSeconds());

            float dx = playerPos.x - mPosition.x;
            float dy = playerPos.y - mPosition.y;
            float angle = std::atan2(dy, dx);

            if (std::abs(angle) < M_PI / 4.0f)
            {
                mAnimationState = AnimationState::WALK_FORWARD;
            }
            else if (std::abs(angle - M_PI / 2.0f) < M_PI / 4.0f)
            {
                mAnimationState = AnimationState::WALK_RIGHT;
            }
            else if (std::abs(angle - M_PI) < M_PI / 4.0f)
            {
                mAnimationState = AnimationState::WALK_BACK;
            }
            else if (std::abs(angle + M_PI / 2.0f) < M_PI / 4.0f)
            {
                mAnimationState = AnimationState::WALK_LEFT;
            }
            else
            {
                mAnimationState = AnimationState::WALK_FORWARD; // fallback
            }
        }
        else
        {
            mAnimationState = AnimationState::IDLE;
        }
    }
    else
    {
        mAnimationState = AnimationState::IDLE;
    }

    updateState(deltaTime, playerPos);

    bool hasLineOfSight = canSeePlayer(playerPos);

    float targetVisibility = hasLineOfSight ? 1.0f : 0.75f;
    float visibilitySpeed = 3.0f;

    if (mVisibility < targetVisibility)
    {
        mVisibility += visibilitySpeed * deltaTime.asSeconds();
        if (mVisibility > targetVisibility)
            mVisibility = targetVisibility;
    }
    else if (mVisibility > targetVisibility)
    {
        mVisibility -= visibilitySpeed * deltaTime.asSeconds();
        if (mVisibility < targetVisibility)
            mVisibility = targetVisibility;
    }

    mVisibility = std::max(0.0f, std::min(mVisibility, 1.0f));

    if (mFrameTimer.getElapsedTime().asSeconds() > mFrameRate)
    {
        mFrameTimer.restart();
        mCurrentFrame = (mCurrentFrame + 1) % 4; // 4 кадра на состояние
    }
}

void Enemy::updateState(sf::Time deltaTime, const sf::Vector2f &playerPos)
{
    float distance = std::sqrt(
        std::pow(playerPos.x - mPosition.x, 2) +
        std::pow(playerPos.y - mPosition.y, 2));

    if (distance < mStats.attackRange)
    {
        mState = EnemyState::ATTACK;
        mAnimationState = AnimationState::ATTACK;
    }
    else if (distance < mDetectionDistance)
    {
        mState = EnemyState::CHASE;
    }
    else if (mState == EnemyState::CHASE)
    {
        mState = EnemyState::PATROL;
    }
}

void Enemy::patrol(sf::Time deltaTime)
{
    if (mPatrolTarget == sf::Vector2f(0, 0))
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(-5.0f, 5.0f);

        mPatrolTarget = mPosition + sf::Vector2f(dist(gen), dist(gen));
    }

    float dx = mPatrolTarget.x - mPosition.x;
    float dy = mPatrolTarget.y - mPosition.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 0.1f)
    {
        dx /= distance;
        dy /= distance;
        float newX = mPosition.x + dx * mSpeed * 0.3f * deltaTime.asSeconds();
        float newY = mPosition.y + dy * mSpeed * 0.3f * deltaTime.asSeconds();

        if (!mMap.isWall(newX, newY))
        {
            mPosition.x = newX;
            mPosition.y = newY;
            mAnimationState = AnimationState::WALK_FORWARD;
        }
    }
    else
    {
        mPatrolTarget = sf::Vector2f(0, 0);
        mAnimationState = AnimationState::IDLE;
    }
}

void Enemy::attackPlayer(Player &player, sf::Time deltaTime)
{
    if (!mAlive)
        return;

    mAttackTimer += deltaTime;

    if (mAttackTimer >= mAttackCooldown)
    {
        float dx = player.getX() - mPosition.x;
        float dy = player.getY() - mPosition.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < mStats.attackRange)
        {
            player.takeDamage(mStats.damage);
            mAttackTimer = sf::Time::Zero;
        }
    }
}

void Enemy::render2D(sf::RenderTarget &target) const
{
    if (!mAlive)
        return;

    sf::CircleShape shape(0.2f);

    switch (mAnimationState)
    {
    case AnimationState::ATTACK:
        shape.setFillColor(sf::Color::Magenta);
        break;
    case AnimationState::IDLE:
        shape.setFillColor(sf::Color::Red);
        break;
    default:
        shape.setFillColor(sf::Color(255, 100, 100));
    }

    shape.setPosition(mPosition.x - 0.1f, mPosition.y - 0.1f);
    target.draw(shape);
}

bool Enemy::isVisible(const sf::Vector2f &playerPos, const RayCalc &rayCalc) const
{
    if (!mAlive)
        return false;

    class SimpleObject : public GameObject
    {
    public:
        sf::Vector2f position;
        SimpleObject(sf::Vector2f pos) : position(pos) {}
        sf::Vector2f getPosition() const override { return position; }
        void update(float deltaTime) override {}
        void draw(sf::RenderTarget &target) const override {}
        sf::FloatRect getBoundingBox() const override
        {
            return sf::FloatRect(position.x - 0.1f, position.y - 0.1f, 0.2f, 0.2f);
        }
    };

    SimpleObject tempObj(mPosition);
    ObjectVisibilityData data = rayCalc.getObjectVisibility(&tempObj);
    return data.isVisible;
}

void Enemy::render3D(Pseudo3DRenderer &renderer, const sf::Vector2f &playerPos,
                     const RayCalc &rayCalc) const
{
    if (!mAlive || mVisibility <= 0.01f)
        return;

    if (!isVisible(playerPos, rayCalc))
        return;

    const sf::Texture *enemyTex = (getType() == EnemyType::Boss)
                                      ? renderer.getBossTexture()
                                      : renderer.getEnemyTexture();
    if (!enemyTex)
        return;

    float dx = playerPos.x - mPosition.x;
    float dy = playerPos.y - mPosition.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    const float maxRenderDistance = 40.0f;
    if (distance > maxRenderDistance)
        return;

    sf::IntRect region;
    if (getType() == EnemyType::Boss)
    {
        const sf::Vector2u texSize = enemyTex->getSize();
        const int frameWidth = static_cast<int>(texSize.x / 2);
        const int frameHeight = static_cast<int>(texSize.y / 3);

        int row = 0;
        if (mAnimationState == AnimationState::ATTACK)
            row = 1;
        else if (mAnimationState == AnimationState::DEATH || !mAlive)
            row = 2;

        int col = (mCurrentFrame / 2) % 2;
        region.left = col * frameWidth;
        region.top = row * frameHeight;
        region.width = frameWidth;
        region.height = frameHeight;
    }
    else
    {
        int col = mCurrentFrame % 8;
        region.left = col * 128;
        region.top = 0;
        region.width = 128;
        region.height = 128;
    }

    sf::Color tint = sf::Color::White;
    if (getType() == EnemyType::Raider)
    {
        tint = sf::Color(140, 220, 140);
    }
    else if (getType() == EnemyType::Boss)
    {
        tint = sf::Color(220, 120, 120);
    }

    float flash = getHitFlashIntensity();
    if (flash > 0.0f)
    {
        tint.r = static_cast<sf::Uint8>(std::min(255.0f, tint.r + flash * 110.0f));
        tint.g = static_cast<sf::Uint8>(std::max(0.0f, tint.g - flash * 60.0f));
        tint.b = static_cast<sf::Uint8>(std::max(0.0f, tint.b - flash * 60.0f));
    }

    renderer.renderSprite(mPosition, playerPos, enemyTex, region, 0.0f, mVisibility, mSpriteWorldHeight, tint);
}

void Enemy::setAnimationState(AnimationState state)
{
    mAnimationState = state;
}

void Enemy::takeDamage(int amount)
{
    if (!mAlive || amount <= 0)
        return;

    mHitFlashTimer = 0.18f;

    int remaining = amount;
    if (mStats.armor > 0)
    {
        int absorbed = std::min(mStats.armor, remaining);
        mStats.armor -= absorbed;
        remaining -= absorbed;
    }

    if (remaining > 0)
    {
        mStats.health = std::max(0, mStats.health - remaining);
        if (mStats.health == 0)
        {
            mAlive = false;
            mAnimationState = AnimationState::DEATH;
        }
    }
}
