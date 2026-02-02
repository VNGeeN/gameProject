#pragma once

#include <SFML/Graphics.hpp>
#include "../Map/Map.h"

class Player
{
public:
    struct Stats
    {
        int maxHealth = 120;
        int health = 120;
        int maxArmor = 80;
        int armor = 80;
        int strength = 12;
        int discipline = 10;
        int tech = 8;
    };

    struct WeaponStats
    {
        int damage = 18;
        float range = 14.0f;
        float fireRate = 0.35f;
        float aimCone = 0.08f;
    };

    Player(Map &map);

    float getX() const { return x; }
    float getY() const { return y; }
    float getAngle() const { return angle; }

    void setPosition(float newX, float newY)
    {
        x = newX;
        y = newY;
    }

    void moveForward(float distance);
    void moveBackward(float distance);
    void rotate(float angle);

    const Stats& getStats() const { return mStats; }
    const WeaponStats& getWeaponStats() const { return mWeapon; }
    bool isAlive() const { return mStats.health > 0; }
    void takeDamage(int amount);

    bool tryFire(sf::Time deltaTime);
    void updateWeaponCooldown(sf::Time deltaTime);

    float fov;

private:
    float x, y;
    float angle;
    Map &map;

    Stats mStats;
    WeaponStats mWeapon;
    float mWeaponCooldown = 0.0f;
};
