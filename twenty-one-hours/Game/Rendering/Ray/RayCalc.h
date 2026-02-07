#pragma once
#include <vector>
#include <cmath>
#include <cfloat>
#include "../Data/ObjectVisibilityData.h"
#include "../Map/Map.h"
#include "../Player/Player.h"
#include "../Entities/Surface.h"

class RayCalc
{
public:
    struct Ray
    {
        float distance;
        bool hitWall;
        Surface *hitSurface;
        float hitX, hitY;
        int mapX, mapY;
        int side;

        int segmentX, segmentY;
        sf::Color segmentColor;

        Ray() : distance(0), hitWall(false), hitSurface(nullptr),
                hitX(0), hitY(0), mapX(0), mapY(0), side(0),
                segmentX(-1), segmentY(-1), segmentColor(sf::Color::White) {}
    };

    RayCalc(Player &player, Map &map);
    void calcRays(int rayCount);
    Ray calcSingleRay(float rayAngle) const;
    const std::vector<Ray> &getRays() const { return rays; }

    ObjectVisibilityData getObjectVisibility(GameObject *obj) const;

private:
    std::vector<Ray> rays;
    Player &player;
    Map &map;

    static constexpr float MAX_VIEW_DISTANCE = 55.0f;
};