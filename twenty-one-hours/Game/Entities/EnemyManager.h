#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Enemy.h"

class Player;
class Map;
class Pseudo3DRenderer;

class EnemyManager
{
public:
    EnemyManager(Map &map);

    void addEnemy(std::unique_ptr<Enemy> enemy);
    void spawnEnemies(int count);

    void update(sf::Time deltaTime, Player &player);
    void render2D(sf::RenderTarget &target) const;
    void render3D(Pseudo3DRenderer &renderer, const Player &player,
                  const RayCalc &rayCalc) const;

    size_t getEnemyCount() const { return mEnemies.size(); }

private:
    Map &mMap;
    std::vector<std::unique_ptr<Enemy>> mEnemies;
};