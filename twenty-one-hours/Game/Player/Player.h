#pragma once

#include <SFML/Graphics.hpp>
#include "../Map/Map.h"

class Player
{
public:
    Player(Map& map);

    float getX() const { return x; }
    float getY() const { return y; }
    float getAngle() const { return angle; }

    void setPosition(float newX, float newY) { x = newX; y = newY; }

    void moveForward(float distance);
    void moveBackward(float distance);
    void rotate(float angle);

    float fov;

private:
    float x, y;
    float angle;
    Map& map;
};
