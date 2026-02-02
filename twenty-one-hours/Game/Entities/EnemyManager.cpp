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

    for (int i = 0; i < count; i++)
    {
        int attempts = 0;
        bool spawned = false;

        while (!spawned && attempts < 50)
        {
            int x = xDist(gen);
            int y = yDist(gen);

            // Проверяем, что это пол, а не стена
            if (!mMap.isWall(static_cast<float>(x) + 0.5f,
                             static_cast<float>(y) + 0.5f))
            {

                // Проверяем расстояние до других врагов
                bool tooClose = false;
                for (const auto &enemy : mEnemies)
                {
                    float dx = enemy->getPosition().x - (x + 0.5f);
                    float dy = enemy->getPosition().y - (y + 0.5f);
                    if (dx * dx + dy * dy < 4.0f)
                    { // Минимум 2 клетки между врагами
                        tooClose = true;
                        break;
                    }
                }

                if (!tooClose)
                {
                    mEnemies.push_back(std::make_unique<Enemy>(
                        mMap,
                        static_cast<float>(x) + 0.5f,
                        static_cast<float>(y) + 0.5f));
                    spawned = true;
                    std::cout << "[EnemyManager] Spawned enemy at ("
                              << x << ", " << y << ")" << std::endl;
                }
            }
            attempts++;
        }
    }
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

    // Удаление мёртвых
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
                            const RayCalc &rayCalc) const // Добавьте const
{
    sf::Vector2f playerPos(player.getX(), player.getY());

    // Сортируем врагов по расстоянию для правильного порядка отрисовки
    std::vector<std::pair<float, Enemy *>> sortedEnemies;
    for (const auto &enemy : mEnemies)
    {
        float dx = enemy->getPosition().x - playerPos.x;
        float dy = enemy->getPosition().y - playerPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        sortedEnemies.push_back({distance, enemy.get()});
    }

    // Сортируем по убыванию расстояния (рисуем дальних первыми)
    std::sort(sortedEnemies.begin(), sortedEnemies.end(),
              [](const auto &a, const auto &b)
              { return a.first > b.first; });

    for (const auto &[distance, enemy] : sortedEnemies)
    {
        enemy->render3D(renderer, playerPos, rayCalc);
    }
}