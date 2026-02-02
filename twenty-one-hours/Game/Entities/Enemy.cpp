#include "Enemy.h"
#include "../Rendering/Render/Pseudo3DRenderer.h"
#include "../Ray/RayCalc.h"
#include <cmath>
#include <random>
#include <algorithm>
#include "../Player/Player.h"

Enemy::Enemy(Map &map, float x, float y)
    : mMap(map), mPosition(x, y), mAttackCooldown(sf::seconds(1.0f)),
      mAttackTimer(sf::Time::Zero), mStateTimer(sf::Time::Zero) {}

bool Enemy::canSeePlayer(const sf::Vector2f &playerPos) const
{
    float dx = playerPos.x - mPosition.x;
    float dy = playerPos.y - mPosition.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > mDetectionDistance)
        return false;

    // Проверяем, нет ли стен на пути
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

    if (distance > 0.5f) // Минимальное расстояние - не подходим вплотную
    {
        dx /= distance;
        dy /= distance;
        float newX = mPosition.x + dx * mSpeed * deltaTime;
        float newY = mPosition.y + dy * mSpeed * deltaTime;

        // Проверяем коллизию с картой
        if (!mMap.isWall(newX, newY))
        {
            // Проверяем коллизию с игроком (радиус 0.8)
            float playerDistance = sqrt(
                (newX - playerPos.x) * (newX - playerPos.x) +
                (newY - playerPos.y) * (newY - playerPos.y));

            if (playerDistance > 0.8f) // Минимальное расстояние до игрока
            {
                mPosition.x = newX;
                mPosition.y = newY;
            }
            // Если слишком близко к игроку - останавливаемся
        }
    }
}

void Enemy::update(sf::Time deltaTime, const sf::Vector2f &playerPos, float playerAngle)
{
    mLastPosition = mPosition;
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

    // LOD
    if (distanceToPlayer > 15.0f && mFrameCounter % 3 != 0)
        return;

    // Фрустум
    // УБИРАЕМ ПОВТОРНОЕ ОБЪЯВЛЕНИЕ float angleToEnemy и float angleDiff
    // Используем те же переменные, что объявлены выше в методе
    float angleToEnemy = std::atan2(mPosition.y - playerPos.y, mPosition.x - playerPos.x);
    float angleDiff = std::abs(angleToEnemy - playerAngle);
    if (angleDiff > M_PI)
        angleDiff = 2 * M_PI - angleDiff;

    if (angleDiff < M_PI / 2.0f || distanceToPlayer < 8.0f)
    {
        if (canSeePlayer(playerPos))
        {
            moveToPlayer(playerPos, deltaTime.asSeconds());

            // Определяем направление движения
            float dx = playerPos.x - mPosition.x;
            float dy = playerPos.y - mPosition.y;
            float angle = std::atan2(dy, dx);

            // Выбираем анимацию по углу
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

    // Плавное изменение видимости
    // УБИРАЕМ ПОВТОРНОЕ ОБЪЯВЛЕНИЕ - используем уже вычисленные значения
    // В пределах поля зрения?
    bool inFOV = (angleDiff < ((M_PI / 3.0f) / 2.0f));

    // Плавное изменение видимости
    float targetVisibility = inFOV ? 1.0f : 0.0f;
    float visibilitySpeed = 3.0f; // Скорость появления/исчезания

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

    // Анимация
    if (mFrameTimer.getElapsedTime().asSeconds() > mFrameRate)
    {
        mFrameTimer.restart();
        mCurrentFrame = (mCurrentFrame + 1) % 4; // 4 кадра на состояние
    }
}

// void Enemy::update(sf::Time deltaTime, const sf::Vector2f &playerPos, float playerAngle)
// {
//     mLastPosition = mPosition;
//     if (!mAlive)
//     {
//         // Умирающая анимация
//         mAnimationState = AnimationState::DEATH;
//         if (mFrameTimer.getElapsedTime().asSeconds() > mFrameRate)
//         {
//             mFrameTimer.restart();
//             mCurrentFrame = (mCurrentFrame + 1) % 4;
//         }
//         return;
//     }

//     mFrameCounter++;
//     float distanceToPlayer = std::sqrt(
//         std::pow(playerPos.x - mPosition.x, 2) +
//         std::pow(playerPos.y - mPosition.y, 2));

//     // LOD
//     if (distanceToPlayer > 15.0f && mFrameCounter % 3 != 0)
//         return;

//     // Фрустум
//     float angleToEnemy = std::atan2(mPosition.y - playerPos.y, mPosition.x - playerPos.x);
//     float angleDiff = std::abs(angleToEnemy - playerAngle);
//     if (angleDiff > M_PI)
//         angleDiff = 2 * M_PI - angleDiff;

//     if (angleDiff < M_PI / 2.0f || distanceToPlayer < 8.0f)
//     {
//         if (canSeePlayer(playerPos))
//         {
//             moveToPlayer(playerPos, deltaTime.asSeconds());

//             // Определяем направление движения относительно игрока
//             float dx = playerPos.x - mPosition.x;
//             float dy = playerPos.y - mPosition.y;
//             float angle = std::atan2(dy, dx);

//             // Выбираем анимацию по углу
//             if (std::abs(angle) < M_PI / 4.0f)
//             {
//                 mAnimationState = AnimationState::WALK_FORWARD;
//                 mFacingDirection = FacingDirection::FORWARD;
//             }
//             else if (std::abs(angle - M_PI / 2.0f) < M_PI / 4.0f)
//             {
//                 mAnimationState = AnimationState::WALK_RIGHT;
//                 mFacingDirection = FacingDirection::RIGHT;
//             }
//             else if (std::abs(angle - M_PI) < M_PI / 4.0f)
//             {
//                 mAnimationState = AnimationState::WALK_BACK;
//                 mFacingDirection = FacingDirection::BACK;
//             }
//             else if (std::abs(angle + M_PI / 2.0f) < M_PI / 4.0f)
//             {
//                 mAnimationState = AnimationState::WALK_LEFT;
//                 mFacingDirection = FacingDirection::LEFT;
//             }
//         }
//         else
//         {
//             mAnimationState = AnimationState::IDLE;
//         }
//     }
//     else
//     {
//         mAnimationState = AnimationState::IDLE;
//     }

//     // Анимация
//     if (mFrameTimer.getElapsedTime().asSeconds() > mFrameRate)
//     {
//         mFrameTimer.restart();
//         mCurrentFrame = (mCurrentFrame + 1) % 4; // 4 кадра на состояние
//     }

//      // Плавное изменение видимости
//     angleToEnemy = std::atan2(mPosition.y - playerPos.y, mPosition.x - playerPos.x);
//     angleDiff = std::abs(angleToEnemy - playerAngle);
//     if (angleDiff > M_PI) angleDiff = 2 * M_PI - angleDiff;

//     // В пределах поля зрения?
//     bool inFOV = (angleDiff < ((M_PI / 3.0f) / 2.0f));

//     // Плавное изменение видимости
//     float targetVisibility = inFOV ? 1.0f : 0.0f;
//     float visibilitySpeed = 3.0f; // Скорость появления/исчезания

//     if (mVisibility < targetVisibility) {
//         mVisibility += visibilitySpeed * deltaTime.asSeconds();
//         if (mVisibility > targetVisibility) mVisibility = targetVisibility;
//     } else if (mVisibility > targetVisibility) {
//         mVisibility -= visibilitySpeed * deltaTime.asSeconds();
//         if (mVisibility < targetVisibility) mVisibility = targetVisibility;
//     }

//     // Также учитываем расстояние
//     float distance = sqrt(
//         std::pow(playerPos.x - mPosition.x, 2) +
//         std::pow(playerPos.y - mPosition.y, 2)
//     );

//     // Дальние враги менее видны
//     float distanceVisibility = 1.0f - (distance / 20.0f);
//     if (distanceVisibility < 0.0f) distanceVisibility = 0.0f;

//     mVisibility *= distanceVisibility;
// }

void Enemy::updateState(sf::Time deltaTime, const sf::Vector2f &playerPos)
{
    float distance = std::sqrt(
        std::pow(playerPos.x - mPosition.x, 2) +
        std::pow(playerPos.y - mPosition.y, 2));

    if (distance < 1.5f)
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
        // Потерял игрока из виду
        mState = EnemyState::PATROL;
    }
}

void Enemy::patrol(sf::Time deltaTime)
{
    // Если нет цели патрулирования, выбираем новую
    if (mPatrolTarget == sf::Vector2f(0, 0))
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(-5.0f, 5.0f);

        mPatrolTarget = mPosition + sf::Vector2f(dist(gen), dist(gen));
    }

    // Двигаемся к цели
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
        // Достигли цели, выбираем новую
        mPatrolTarget = sf::Vector2f(0, 0);
        mAnimationState = AnimationState::IDLE;
    }
}

void Enemy::attackPlayer(Player &player, sf::Time deltaTime)
{
    mAttackTimer += deltaTime;

    if (mAttackTimer >= mAttackCooldown)
    {
        // Проверяем расстояние до игрока
        float dx = player.getX() - mPosition.x;
        float dy = player.getY() - mPosition.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < 1.5f)
        {
            // Наносим урон игроку
            // player.takeDamage(mDamage);
            mAttackTimer = sf::Time::Zero;
        }
    }
}

void Enemy::render2D(sf::RenderTarget &target) const
{
    if (!mAlive)
        return;

    sf::CircleShape shape(0.2f);

    // Цвет в зависимости от состояния
    switch (mAnimationState)
    {
    case AnimationState::ATTACK:
        shape.setFillColor(sf::Color::Magenta);
        break;
    case AnimationState::IDLE:
        shape.setFillColor(sf::Color::Red);
        break;
    default:
        shape.setFillColor(sf::Color(255, 100, 100)); // Светло-красный для движения
    }

    shape.setPosition(mPosition.x - 0.1f, mPosition.y - 0.1f);
    target.draw(shape);
}

bool Enemy::isVisible(const sf::Vector2f &playerPos, const RayCalc &rayCalc) const
{
    if (!mAlive)
        return false;

    // Создаем простой объект для проверки видимости
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

    const sf::Texture *enemyTex = renderer.getEnemyTexture();
    if (!enemyTex)
        return;

    float dx = playerPos.x - mPosition.x;
    float dy = playerPos.y - mPosition.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 20.0f && mFrameCounter % 4 != 0)
        return;

    // Упрощенная анимация: одна строка с 8 кадрами
    sf::IntRect region;
    int col = mCurrentFrame % 8; // 8 кадров в атласе
    region.left = col * 128;
    region.top = 0;
    region.width = 128;
    region.height = 128;

    renderer.renderSprite(mPosition, playerPos, enemyTex, region, 0.0f, mVisibility);
}

void Enemy::setAnimationState(AnimationState state)
{
    mAnimationState = state;
}
