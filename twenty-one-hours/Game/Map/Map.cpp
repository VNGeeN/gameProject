#include "Map.h"
#include <cmath>
#include <iostream>

Map::Map()
{
    mWidth = 10;
    mHeight = 10;
    mGrid = {
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '#', '#', '.', '#', '#', '.', '.', '#'},
        {'#', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
        {'#', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
        {'#', '.', '.', '.', '#', '.', '.', '.', '.', '#'},
        {'#', '.', '#', '.', '#', '.', '#', '.', '.', '#'},
        {'#', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}};

}

int Map::getWidth() const
{
    return mWidth;
}

int Map::getHeight() const
{
    return mHeight;
}

char Map::getTitle(int x, int y) const
{
    if (x >= 0 && x < mWidth && y >= 0 && y < mHeight)
    {
        return mGrid[y][x];
    }
    return '#';
}

bool Map::isWall(float x, float y) const
{
    int mapX = static_cast<int>(x);
    int mapY = static_cast<int>(y);

    if (mapX >= 0 && mapX < mWidth && mapY >= 0 && mapY < mHeight)
    {
        return mGrid[mapY][mapX] == '#';
    }
    return true; // Вне карты считаем стеной
}

GameObject *Map::getObjectAt(float x, float y)
{
    for (auto &obj : mObjects)
    {
        sf::Vector2f pos = obj->getPosition();
        float dx = pos.x - x;
        float dy = pos.y - y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < 0.5f)
        { // Если объект достаточно близко к координатам
            return obj.get();
        }
    }
    return nullptr;
}

std::vector<GameObject *> Map::getObjects() const
{
    std::vector<GameObject *> result;
    for (const auto &obj : mObjects)
    {
        result.push_back(obj.get());
    }
    return result;
}

void Map::addObject(std::unique_ptr<GameObject> object)
{
    mObjects.push_back(std::move(object));
}