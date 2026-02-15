#pragma once

#include <SFML/Graphics.hpp>
#include "../Map/Map.h"
#include <array>
#include <string>

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
        std::string name;
        int damage = 18;
        float range = 14.0f;
        float fireRate = 0.35f;
        float aimCone = 0.08f;
        int magazineSize = 12;
        int ammoInMagazine = 12;
        int reserveAmmo = 36;
        int ammoPerPickup = 12;
    };

    enum class WeaponType
    {
        Pistol = 0,
        Rifle = 1,
        Shotgun = 2,
        Count
    };

    Player(Map &map);

    float getX() const { return x; }
    float getY() const { return y; }
    float getAngle() const { return angle; }
    float getPitch() const { return pitch; }

    void setPosition(float newX, float newY)
    {
        x = newX;
        y = newY;
    }

    void moveForward(float distance);
    void moveBackward(float distance);
    void strafeLeft(float distance);
    void strafeRight(float distance);
    void rotate(float angle);
    void rotatePitch(float pitchOffset);

    const Stats &getStats() const { return mStats; }
    const WeaponStats &getWeaponStats() const { return mWeapons[static_cast<std::size_t>(mCurrentWeapon)]; }
    WeaponType getCurrentWeaponType() const { return mCurrentWeapon; }
    const char *getCurrentWeaponName() const { return getWeaponStats().name.c_str(); }
    bool isAlive() const { return mStats.health > 0; }
    void takeDamage(int amount);

    bool tryFire(sf::Time deltaTime);
    void updateWeaponCooldown(sf::Time deltaTime);
    bool reloadActiveWeapon();
    void addAmmoToAllWeapons();
    bool switchWeapon(int weaponIndex);
    void updateRegeneration(sf::Time deltaTime);
    void resetAfterDeath();

    float fov;

private:
    float x, y;
    float angle;
    float pitch;
    Map &map;

    Stats mStats;
    std::array<WeaponStats, static_cast<std::size_t>(WeaponType::Count)> mWeapons;
    WeaponType mCurrentWeapon = WeaponType::Pistol;
    float mWeaponCooldown = 0.0f;
    float mHealthRegenTimer = 0.0f;
    float mArmorRegenTimer = 0.0f;
};
