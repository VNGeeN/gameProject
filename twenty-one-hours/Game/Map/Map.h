#pragma once

#include <vector>
#include <memory>
#include "../GameObject/GameObject.h"

class Map
{
public:
    Map();
    int getWidth() const;
    int getHeight() const;
    char getTitle(int x, int y) const;

    bool isWall(float x, float y) const;
    GameObject *getObjectAt(float x, float y);
    std::vector<GameObject *> getObjects() const;

    // Временная реализация - позже можно добавить настоящие объекты
    void addObject(std::unique_ptr<GameObject> object);

private:
    std::vector<std::vector<char>> mGrid;
    int mWidth, mHeight;
    std::vector<std::unique_ptr<GameObject>> mObjects;
};