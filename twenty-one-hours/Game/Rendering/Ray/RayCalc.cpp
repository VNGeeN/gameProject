#include "RayCalc.h"
#include "../Spatial/Utils.h"
#include <iostream>
#include <algorithm>

RayCalc::RayCalc(Player &p, Map &m) : player(p), map(m)
{
}

void RayCalc::calcRays(int rayCount)
{
    rays.clear();
    rays.reserve(rayCount);

    float posX = player.getX();
    float posY = player.getY();
    float angle = player.getAngle();

    if (rayCount <= 0)
    {
        return;
    }

    float rayStep = (rayCount == 1)
                        ? 0.0f
                        : (player.fov / static_cast<float>(rayCount - 1));

    for (int i = 0; i < rayCount; i++)
    {
        float rayAngle = angle - (player.fov / 2.0f) + (rayStep * i);

        Ray ray = calcSingleRay(rayAngle);
        rays.push_back(ray);
    }
}

RayCalc::Ray RayCalc::calcSingleRay(float rayAngle) const
{
    Ray ray;
    ray.hitWall = false;
    ray.hitSurface = nullptr;
    ray.distance = MAX_VIEW_DISTANCE;
    ray.segmentX = -1;
    ray.segmentY = -1;

    float posX = player.getX();
    float posY = player.getY();

    rayAngle = fmod(rayAngle, 2.0f * static_cast<float>(M_PI));
    if (rayAngle < 0)
        rayAngle += 2.0f * static_cast<float>(M_PI);

    float rayDirX = cos(rayAngle);
    float rayDirY = sin(rayAngle);

    int mapX = static_cast<int>(posX);
    int mapY = static_cast<int>(posY);

    float deltaDistX = (rayDirX == 0) ? 1e30f : std::abs(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30f : std::abs(1.0f / rayDirY);

    float sideDistX, sideDistY;
    int stepX, stepY;
    int side;

    if (rayDirX < 0)
    {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (mapX + 1.0f - posX) * deltaDistX;
    }

    if (rayDirY < 0)
    {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (mapY + 1.0f - posY) * deltaDistY;
    }

    bool hit = false;
    int steps = 0;
    const int MAX_STEPS = 100;

    Surface *initialSurface = map.getSurfaceAt(
        static_cast<float>(mapX),
        static_cast<float>(mapY),
        Surface::Type::WALL);
    if (initialSurface)
    {
        float localX = posX - std::floor(posX);
        float localY = posY - std::floor(posY);
        int segX = utils::clamp(static_cast<int>(localX * 3), 0, 2);
        int segY = utils::clamp(static_cast<int>(localY * 3), 0, 2);

        if (!initialSurface->isSegmentPassable(segX, segY))
        {
            ray.distance = 0.01f;
            ray.hitX = posX;
            ray.hitY = posY;
            ray.mapX = mapX;
            ray.mapY = mapY;
            ray.side = 0;
            ray.hitSurface = initialSurface;
            ray.segmentX = segX;
            ray.segmentY = segY;
            ray.segmentColor = initialSurface->getColorAtSegment(segX, segY, ray.distance);
            ray.hitWall = true;
            return ray;
        }
    }

    while (!hit && steps < MAX_STEPS)
    {
        steps++;

        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        }
        else
        {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }

        if (mapX < 0 || mapX >= map.getWidth() ||
            mapY < 0 || mapY >= map.getHeight())
        {
            hit = true;
            ray.hitWall = true;
            ray.distance = MAX_VIEW_DISTANCE;
            break;
        }

        Surface *surface = map.getSurfaceAt(
            static_cast<float>(mapX),
            static_cast<float>(mapY),
            Surface::Type::WALL);

        if (surface)
        {
            float rawDistance;
            if (side == 0)
            {
                rawDistance = (mapX - posX + (1 - stepX) / 2.0f) / rayDirX;
            }
            else
            {
                rawDistance = (mapY - posY + (1 - stepY) / 2.0f) / rayDirY;
            }

            if (rawDistance < 0.01f)
                rawDistance = 0.01f;

            float angleDiff = std::abs(rayAngle - player.getAngle());
            if (angleDiff > static_cast<float>(M_PI))
                angleDiff = 2.0f * static_cast<float>(M_PI) - angleDiff;
            ray.distance = rawDistance * cos(angleDiff);
            if (ray.distance < 0.02f)
                ray.distance = 0.02f;

            ray.hitX = posX + rayDirX * rawDistance;
            ray.hitY = posY + rayDirY * rawDistance;
            ray.mapX = mapX;
            ray.mapY = mapY;
            ray.side = side;
            ray.hitSurface = surface;

            float localX = ray.hitX - std::floor(ray.hitX);
            float localY = ray.hitY - std::floor(ray.hitY);

            int segX = -1, segY = -1;

            if (side == 0)
            {
                segY = utils::clamp(static_cast<int>(localY * 3), 0, 2);
                segX = (stepX > 0) ? 0 : 2;
            }
            else
            {
                segX = utils::clamp(static_cast<int>(localX * 3), 0, 2);
                segY = (stepY > 0) ? 0 : 2;
            }

            ray.segmentX = segX;
            ray.segmentY = segY;

            if (surface->isSegmentPassable(segX, segY))
            {
                continue;
            }

            ray.segmentColor = surface->getColorAtSegment(
                segX, segY, ray.distance);

            hit = true;
            ray.hitWall = true;
        }
    }

    return ray;
}

ObjectVisibilityData RayCalc::getObjectVisibility(GameObject *obj) const
{
    ObjectVisibilityData data;
    data.isVisible = false;
    data.distance = FLT_MAX;

    sf::Vector2f objPos = obj->getPosition();
    float dx = objPos.x - player.getX();
    float dy = objPos.y - player.getY();
    float distance = std::sqrt(dx * dx + dy * dy);

    float angleToObj = atan2(dy, dx);
    float angleDiff = std::abs(angleToObj - player.getAngle());
    angleDiff = std::min(angleDiff, 2.0f * static_cast<float>(M_PI) - angleDiff);

    if (angleDiff > player.fov / 2.0f)
        return data;

    Ray toObj = calcSingleRay(angleToObj);
    float angleDiffForRay = std::abs(angleToObj - player.getAngle());
    angleDiffForRay = std::min(angleDiffForRay, 2.0f * static_cast<float>(M_PI) - angleDiffForRay);
    float cosAngle = std::cos(angleDiffForRay);
    float rawRayDistance = (std::abs(cosAngle) > 0.0001f)
                               ? (toObj.distance / cosAngle)
                               : toObj.distance;

    if (rawRayDistance < distance - 0.1f)
        return data;

    if (distance < 5.0f)
    {
        data.isVisible = true;
        data.distance = distance;
    }

    return data;
}