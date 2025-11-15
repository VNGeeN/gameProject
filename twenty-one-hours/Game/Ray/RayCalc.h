#ifndef RAYCALC_H
#define RAYCALC_H

#include <vector>
#include <cmath>
#include <cfloat>
#include "../GameObject/GameObject.h"
#include "../Player/Player.h"
#include "../Map/Map.h"
#include "../Data/ObjectVisibilityData.h"

class RayCalc
{
public:
    struct Ray
    {
        //float angle;
        float distance;
        bool hitWall;
        GameObject *hitObject;
        float hitX, hitY;
        int mapX, mapY; // Клетка карты где произошло столкновение
        int side;       // Сторона столкновения (0 - x, 1 - y)
        float wallX;    // текстурная координата X
    };

    RayCalc(Player &p, Map &m);
    void calcRays(int rayCount);
    Ray calcSingleRay(float rayAngle);
    const std::vector<Ray> &getRays() const;
    ObjectVisibilityData getObjectVisibility(GameObject *obj);
    

private:
    std::vector<Ray> rays;
    Player &player;
    Map &map;

    static constexpr float MAX_VIEW_DISTANCE = 20.0f;
    static constexpr float STEP_SIZE = 0.05f;
};
#endif