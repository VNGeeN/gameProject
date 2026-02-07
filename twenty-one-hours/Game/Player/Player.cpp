#include "Player.h"
#include <cmath>
#include <iostream>
#include <algorithm>

Player::Player(Map &map) : map(map)
{
    sf::Vector2f startPos = map.findPlayerStartPosition();
    x = startPos.x;
    y = startPos.y;

    angle = 0.0f;
    fov = M_PI / 3.0f;

    WeaponStats pistol;
    pistol.name = "Pistol";
    pistol.damage = 18;
    pistol.range = 14.0f;
    pistol.fireRate = 0.35f;
    pistol.aimCone = 0.08f;
    pistol.magazineSize = 12;
    pistol.ammoInMagazine = 12;
    pistol.reserveAmmo = 36;
    pistol.ammoPerPickup = 12;

    WeaponStats rifle;
    rifle.name = "Rifle";
    rifle.damage = 12;
    rifle.range = 18.0f;
    rifle.fireRate = 0.12f;
    rifle.aimCone = 0.16f;
    rifle.magazineSize = 30;
    rifle.ammoInMagazine = 30;
    rifle.reserveAmmo = 90;
    rifle.ammoPerPickup = 24;

    WeaponStats shotgun;
    shotgun.name = "Shotgun";
    shotgun.damage = 36;
    shotgun.range = 8.0f;
    shotgun.fireRate = 0.75f;
    shotgun.aimCone = 0.28f;
    shotgun.magazineSize = 6;
    shotgun.ammoInMagazine = 6;
    shotgun.reserveAmmo = 24;
    shotgun.ammoPerPickup = 6;

    mWeapons[static_cast<std::size_t>(WeaponType::Pistol)] = pistol;
    mWeapons[static_cast<std::size_t>(WeaponType::Rifle)] = rifle;
    mWeapons[static_cast<std::size_t>(WeaponType::Shotgun)] = shotgun;
}

void Player::moveForward(float distance)
{

    float newX = x + cos(angle) * distance;
    float newY = y + sin(angle) * distance;

    if (!map.isWall(newX, newY))
    {
        x = newX;
        y = newY;
    }
    else
    {
        if (!map.isWall(newX, y))
        {
            x = newX;
        }
        else if (!map.isWall(x, newY))
        {
            y = newY;
        }
    }
}

void Player::moveBackward(float distance)
{
    float newX = x - cos(angle) * distance;
    float newY = y - sin(angle) * distance;

    if (!map.isWall(newX, newY))
    {
        x = newX;
        y = newY;
    }
    else
    {
        if (!map.isWall(newX, y))
        {
            x = newX;
        }
        else if (!map.isWall(x, newY))
        {
            y = newY;
        }
    }
}

void Player::rotate(float angleOffset)
{
    angle += angleOffset;

    if (angle < 0)
    {
        angle += 2 * M_PI;
    }

    if (angle >= 2 * M_PI)
    {
        angle -= 2 * M_PI;
    }
}

void Player::takeDamage(int amount)
{
    if (amount <= 0 || !isAlive())
    {
        return;
    }

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
    }
}

bool Player::tryFire(sf::Time deltaTime)
{
    (void)deltaTime;
    auto &weapon = mWeapons[static_cast<std::size_t>(mCurrentWeapon)];

    if (mWeaponCooldown > 0.0f || weapon.ammoInMagazine <= 0)
    {
        return false;
    }

    weapon.ammoInMagazine = std::max(0, weapon.ammoInMagazine - 1);
    mWeaponCooldown = weapon.fireRate;
    return true;
}

void Player::updateWeaponCooldown(sf::Time deltaTime)
{
    mWeaponCooldown = std::max(0.0f, mWeaponCooldown - deltaTime.asSeconds());
}

bool Player::reloadActiveWeapon()
{
    auto &weapon = mWeapons[static_cast<std::size_t>(mCurrentWeapon)];
    if (weapon.ammoInMagazine >= weapon.magazineSize || weapon.reserveAmmo <= 0)
    {
        return false;
    }

    int needed = weapon.magazineSize - weapon.ammoInMagazine;
    int loaded = std::min(needed, weapon.reserveAmmo);
    weapon.reserveAmmo -= loaded;
    weapon.ammoInMagazine += loaded;
    return loaded > 0;
}

void Player::addAmmoToAllWeapons()
{
    for (auto &weapon : mWeapons)
    {
        weapon.reserveAmmo += weapon.ammoPerPickup;
    }
}

bool Player::switchWeapon(int weaponIndex)
{
    if (weaponIndex < 0 || weaponIndex >= static_cast<int>(WeaponType::Count))
    {
        return false;
    }

    WeaponType nextWeapon = static_cast<WeaponType>(weaponIndex);
    if (nextWeapon == mCurrentWeapon)
    {
        return false;
    }

    mCurrentWeapon = nextWeapon;
    mWeaponCooldown = 0.1f;
    return true;
}