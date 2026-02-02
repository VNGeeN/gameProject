#include "EnemyManager.h"
#include "Enemy.h"
#include "../Player/Player.h"
#include "../Map/Map.h"
#include "../Rendering/Render/Pseudo3DRenderer.h"
#include <cmath>
#include <random>
#include <iostream>

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
            playerPos.x += dx * push;
            playerPos.y += dy * push;
            player.setPosition(playerPos.x, playerPos.y);
        }
    }

    // Удаление мёртвых
    mEnemies.erase(
        std::remove_if(mEnemies.begin(), mEnemies.end(),
            [](const auto& e) { return !e->isAlive(); }),
        mEnemies.end()
    );
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
    std::vector<std::pair<float, Enemy*>> sortedEnemies;
    for (const auto& enemy : mEnemies)
    {
        float dx = enemy->getPosition().x - playerPos.x;
        float dy = enemy->getPosition().y - playerPos.y;
        float distance = std::sqrt(dx*dx + dy*dy);
        sortedEnemies.push_back({distance, enemy.get()});
    }
    
    // Сортируем по убыванию расстояния (рисуем дальних первыми)
    std::sort(sortedEnemies.begin(), sortedEnemies.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    for (const auto& [distance, enemy] : sortedEnemies)
    {
        enemy->render3D(renderer, playerPos, rayCalc);
    }
}