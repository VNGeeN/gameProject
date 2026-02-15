#include "EnemyManager.h"
#include "Enemy.h"
#include "../Player/Player.h"
#include "../Map/Map.h"
#include "../Rendering/Render/Pseudo3DRenderer.h"
#include "../Rendering/Ray/RayCalc.h"
#include <cmath>
#include <random>
#include <iostream>
#include <limits>
#include <algorithm>

EnemyManager::EnemyManager(Map &map) : mMap(map) {}

void EnemyManager::addEnemy(std::unique_ptr<Enemy> enemy)
{
    mEnemies.push_back(std::move(enemy));
}

void EnemyManager::spawnEnemies(int count)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> xDist(1, mMap.getWidth() - 2);
    std::uniform_int_distribution<> yDist(1, mMap.getHeight() - 2);
    std::uniform_real_distribution<float> typeRoll(0.0f, 1.0f);

    for (int i = 0; i < count; i++)
    {
        int attempts = 0;
        bool spawned = false;

        while (!spawned && attempts < 50)
        {
            int x = xDist(gen);
            int y = yDist(gen);

            if (!mMap.isWall(static_cast<float>(x) + 0.5f,
                             static_cast<float>(y) + 0.5f))
            {

                bool tooClose = false;
                for (const auto &enemy : mEnemies)
                {
                    float dx = enemy->getPosition().x - (x + 0.5f);
                    float dy = enemy->getPosition().y - (y + 0.5f);
                    if (dx * dx + dy * dy < 4.0f)
                    { 
                        tooClose = true;
                        break;
                    }
                }

                if (!tooClose)
                {
                    EnemyType type = EnemyType::Grunt;
                    float roll = typeRoll(gen);
                    if (roll > 0.65f)
                    {
                        type = EnemyType::Raider;
                    }

                    mEnemies.push_back(std::make_unique<Enemy>(
                        mMap,
                        static_cast<float>(x) + 0.5f,
                        static_cast<float>(y) + 0.5f,
                        type));
                    spawned = true;
                    std::cout << "[EnemyManager] Spawned enemy at ("
                              << x << ", " << y << ")" << std::endl;
                }
            }
            attempts++;
        }
    }
}

void EnemyManager::spawnBoss()
{
    if (mBossSpawned)
    {
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> xDist(1, mMap.getWidth() - 2);
    std::uniform_int_distribution<> yDist(1, mMap.getHeight() - 2);

    int attempts = 0;
    while (attempts < 100)
    {
        int x = xDist(gen);
        int y = yDist(gen);

        if (!mMap.isWall(static_cast<float>(x) + 0.5f,
                         static_cast<float>(y) + 0.5f))
        {
            bool tooClose = false;
            for (const auto &enemy : mEnemies)
            {
                float dx = enemy->getPosition().x - (x + 0.5f);
                float dy = enemy->getPosition().y - (y + 0.5f);
                if (dx * dx + dy * dy < 9.0f)
                {
                    tooClose = true;
                    break;
                }
            }

            if (!tooClose)
            {
                mEnemies.push_back(std::make_unique<Enemy>(
                    mMap,
                    static_cast<float>(x) + 0.5f,
                    static_cast<float>(y) + 0.5f,
                    EnemyType::Boss));
                mBossSpawned = true;
                std::cout << "[EnemyManager] Spawned boss at ("
                          << x << ", " << y << ")" << std::endl;
                return;
            }
        }
        attempts++;
    }
}

bool EnemyManager::hasBoss() const
{
    if (mBossSpawned)
    {
        return true;
    }

    for (const auto &enemy : mEnemies)
    {
        if (enemy->isBoss())
        {
            return true;
        }
    }

    return false;
}

bool EnemyManager::isBossAlive() const
{
    for (const auto &enemy : mEnemies)
    {
        if (enemy->isBoss() && enemy->isAlive())
        {
            return true;
        }
    }

    return false;
}

void EnemyManager::update(sf::Time deltaTime, Player &player)

{
    sf::Vector2f playerPos(player.getX(), player.getY());
    float playerAngle = player.getAngle();

    for (auto &enemy : mEnemies)
    {
        enemy->update(deltaTime, playerPos, playerAngle);
        enemy->attackPlayer(player, deltaTime);
    }

    const float minSeparation = 0.6f;
    for (const auto &enemy : mEnemies)
    {
        sf::Vector2f enemyPos = enemy->getPosition();
        float dx = playerPos.x - enemyPos.x;
        float dy = playerPos.y - enemyPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 0.0f && distance < minSeparation)
        {
            float push = (minSeparation - distance) / distance;
            sf::Vector2f candidate(playerPos.x + dx * push, playerPos.y + dy * push);

            if (!mMap.isWall(candidate.x, candidate.y))
            {
                playerPos = candidate;
                player.setPosition(playerPos.x, playerPos.y);
            }
            else if (!mMap.isWall(candidate.x, playerPos.y))
            {
                playerPos.x = candidate.x;
                player.setPosition(playerPos.x, playerPos.y);
            }
            else if (!mMap.isWall(playerPos.x, candidate.y))
            {
                playerPos.y = candidate.y;
                player.setPosition(playerPos.x, playerPos.y);
            }
        }
    }

    const float enemySeparation = 0.7f;
    for (size_t i = 0; i < mEnemies.size(); i++)
    {
        for (size_t j = i + 1; j < mEnemies.size(); j++)
        {
            sf::Vector2f posA = mEnemies[i]->getPosition();
            sf::Vector2f posB = mEnemies[j]->getPosition();

            float dx = posB.x - posA.x;
            float dy = posB.y - posA.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 0.0001f && distance < enemySeparation)
            {
                float push = (enemySeparation - distance) / distance * 0.5f;
                posA.x -= dx * push;
                posA.y -= dy * push;
                posB.x += dx * push;
                posB.y += dy * push;

                mEnemies[i]->setPosition(posA);
                mEnemies[j]->setPosition(posB);
            }
        }
    }

    static std::mt19937 dropRng(std::random_device{}());
    static std::uniform_real_distribution<float> dropRoll(0.0f, 1.0f);

    for (const auto &enemy : mEnemies)
    {
        if (!enemy->isAlive() && !enemy->isBoss() && dropRoll(dropRng) <= 0.2f)
        {
            mMap.spawnAmmoPickupAt(enemy->getPosition(), 0.9f);
        }
    }

    mEnemies.erase(
        std::remove_if(mEnemies.begin(), mEnemies.end(),
                       [](const auto &e)
                       { return !e->isAlive(); }),
        mEnemies.end());
}

Enemy *EnemyManager::findTargetInSight(const Player &player, const RayCalc &rayCalc,
                                       float maxDistance, float aimCone) const
{
    Enemy *bestTarget = nullptr;
    float bestScore = std::numeric_limits<float>::max();
    sf::Vector2f playerPos(player.getX(), player.getY());
    float playerAngle = player.getAngle();

    for (const auto &enemy : mEnemies)
    {
        if (!enemy->isAlive())
            continue;

        sf::Vector2f enemyPos = enemy->getPosition();
        float dx = enemyPos.x - playerPos.x;
        float dy = enemyPos.y - playerPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > maxDistance)
            continue;

        float angleToEnemy = std::atan2(dy, dx);
        float angleDiff = std::abs(angleToEnemy - playerAngle);
        if (angleDiff > M_PI)
            angleDiff = 2.0f * static_cast<float>(M_PI) - angleDiff;

        if (angleDiff > aimCone * 0.5f)
            continue;

        RayCalc::Ray ray = rayCalc.calcSingleRay(angleToEnemy);
        float angleDiffForRay = std::abs(angleToEnemy - playerAngle);
        angleDiffForRay = std::min(angleDiffForRay,
                                   2.0f * static_cast<float>(M_PI) - angleDiffForRay);
        float cosAngle = std::cos(angleDiffForRay);
        float rawRayDistance = (std::abs(cosAngle) > 0.0001f)
                                   ? (ray.distance / cosAngle)
                                   : ray.distance;

        if (rawRayDistance + 0.1f < distance)
            continue;

        float score = distance + angleDiff * 6.0f;
        if (score < bestScore)
        {
            bestScore = score;
            bestTarget = enemy.get();
        }
    }

    return bestTarget;
}

void EnemyManager::render2D(sf::RenderTarget &target) const
{
    for (const auto &enemy : mEnemies)
    {
        enemy->render2D(target);
    }
}

void EnemyManager::render3D(Pseudo3DRenderer &renderer, const Player &player,
                            const RayCalc &rayCalc) const 
{
    sf::Vector2f playerPos(player.getX(), player.getY());

    std::vector<std::pair<float, Enemy *>> sortedEnemies;
    for (const auto &enemy : mEnemies)
    {
        float dx = enemy->getPosition().x - playerPos.x;
        float dy = enemy->getPosition().y - playerPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        sortedEnemies.push_back({distance, enemy.get()});
    }

    std::sort(sortedEnemies.begin(), sortedEnemies.end(),
              [](const auto &a, const auto &b)
              { return a.first > b.first; });

    for (const auto &[distance, enemy] : sortedEnemies)
    {
        enemy->render3D(renderer, playerPos, rayCalc);
    }
}