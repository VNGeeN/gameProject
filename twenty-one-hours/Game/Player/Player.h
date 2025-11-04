#pragma once

#include <SFML/Graphics.hpp>

class Player
{
public:
    Player();

    float getX() const { return x; }
    float getY() const { return y; }
    float getAngle() const { return angle; }

    void moveForward(float distance);
    void moveBackward(float distance);
    void rotate(float angle);

    float fov;

private:
    float x, y;
    float angle;
};
