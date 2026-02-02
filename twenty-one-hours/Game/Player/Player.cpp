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
    if (mWeaponCooldown > 0.0f)
    {
        return false;
    }

    mWeaponCooldown = mWeapon.fireRate;
    return true;
}

void Player::updateWeaponCooldown(sf::Time deltaTime)
{
    mWeaponCooldown = std::max(0.0f, mWeaponCooldown - deltaTime.asSeconds());
}