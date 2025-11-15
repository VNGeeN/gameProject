#include "Player.h"
#include <cmath>
#include <iostream>

Player::Player(Map &map) : map(map)
{
    x = 1.5f;
    y = 1.5f;
    angle = 0.0f;      // смотрим вдоль оси X
    fov = M_PI / 3.0f; // 60 градусов
}

void Player::moveForward(float distance)
{

    float newX = x + cos(angle) * distance;
    float newY = y + sin(angle) * distance;
    
    // Проверка коллизий с внутренними стенами
    if (!map.isWall(newX, newY))
    {
        x = newX;
        y = newY;
    }
    else
    {
        // Можно добавить скольжение вдоль стен
        // Пробуем двигаться только по X
        if (!map.isWall(newX, y))
        {
            x = newX;
        }
        // Пробуем двигаться только по Y
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

    // Проверка коллизий с внутренними стенами
    if (!map.isWall(newX, newY))
    {
        x = newX;
        y = newY;
    }
    else
    {
        // Скольжение вдоль стен
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

    // Нормализуем угол
    if (angle < 0)
    {
        angle += 2 * M_PI;
    }

    if (angle >= 2 * M_PI)
    {
        angle -= 2 * M_PI;
    }
}