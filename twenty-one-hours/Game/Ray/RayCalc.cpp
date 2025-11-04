#include "RayCalc.h"
#include <iostream>

RayCalc::RayCalc(Player &p, Map &m) : player(p), map(m)
{
}

// void RayCalc::calcRays(int rayCount)
// {
//     rays.clear();

//     for (int i = 0; i < rayCount; i++)
//     {
//         // Вычисляем угол для каждого луча на основе FOV игрока
//         float cameraX = 2 * i / float(rayCount) - 1; // от -1 до 1
//         float rayDirX = cos(player.getAngle()) + (-sin(player.getAngle())) * player.fov * cameraX;
//         float rayDirY = sin(player.getAngle()) + (cos(player.getAngle())) * player.fov * cameraX;

//         float angle = atan2(rayDirY, rayDirX);
//         Ray ray = calcSingleRay(angle);
//         rays.push_back(ray);
//     }

//     // std::cout << "Calculated " << rays.size() << " rays" << std::endl;
// }

void RayCalc::calcRays(int rayCount)
{
    rays.clear();

    float posX = player.getX();
    float posY = player.getY();
    float angle = player.getAngle();

    std::cout << "Player at: " << posX << ", " << posY << " angle: " << angle << std::endl;

    for (int i = 0; i < rayCount; i++)
    {
        // Вычисляем угол для текущего луча
        float rayAngle = angle - (player.fov / 2.0f) + (player.fov * i / float(rayCount));

        Ray ray = calcSingleRay(rayAngle);
        rays.push_back(ray);
    }

    std::cout << "Calculated " << rays.size() << " rays" << std::endl;
}

// RayCalc::Ray RayCalc::calcSingleRay(float angle)
// {
//     Ray ray;
//     ray.angle = angle;
//     ray.hitWall = false;
//     ray.hitObject = nullptr;
//     ray.distance = MAX_VIEW_DISTANCE;

//     // Параметры игрока
//     float posX = player.getX();
//     float posY = player.getY();
//     float rayDirX = cos(angle);
//     float rayDirY = sin(angle);

//     // Текущая клетка карты
//     int mapX = static_cast<int>(posX);
//     int mapY = static_cast<int>(posY);

//     // Длина луча от одной стороны до другой
//     float deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
//     float deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);

//     float perpWallDist;
//     int stepX, stepY;
//     int hit = 0;
//     int side;

//     float sideDistX, sideDistY;

//     // Вычисляем step и начальное sideDist
//     if (rayDirX < 0)
//     {
//         stepX = -1;
//         sideDistX = (posX - mapX) * deltaDistX;
//     }
//     else
//     {
//         stepX = 1;
//         sideDistX = (mapX + 1.0 - posX) * deltaDistX;
//     }

//     if (rayDirY < 0)
//     {
//         stepY = -1;
//         sideDistY = (posY - mapY) * deltaDistY;
//     }
//     else
//     {
//         stepY = 1;
//         sideDistY = (mapY + 1.0 - posY) * deltaDistY;
//     }

//     // DDA алгоритм
//     while (hit == 0 && ray.distance < MAX_VIEW_DISTANCE)
//     {
//         if (sideDistX < sideDistY)
//         {
//             sideDistX += deltaDistX;
//             mapX += stepX;
//             side = 0;
//         }
//         else
//         {
//             sideDistY += deltaDistY;
//             mapY += stepY;
//             side = 1;
//         }

//         // Проверяем, не вышли ли за границы карты
//         if (mapX < 0 || mapX >= map.getWidth() || mapY < 0 || mapY >= map.getHeight())
//         {
//             hit = 1; // Считаем что попали в стену
//         }
//         else if (map.getTitle(mapX, mapY) == '#')
//         {
//             hit = 1;
//         }

//         // Проверяем столкновение с объектами
//         if (hit == 0)
//         {
//             GameObject *obj = map.getObjectAt(mapX + 0.5f, mapY + 0.5f);
//             if (obj)
//             {
//                 ray.hitObject = obj;
//                 hit = 2; // Отмечаем что попали в объект
//             }
//         }
//     }

//     if (hit > 0)
//     {
//         // Вычисляем расстояние
//         if (side == 0)
//         {
//             perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
//         }
//         else
//         {
//             perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;
//         }

//         ray.distance = perpWallDist;
//         ray.hitWall = (hit == 1);
//         ray.mapX = mapX;
//         ray.mapY = mapY;
//         ray.side = side;
//         ray.hitX = posX + rayDirX * perpWallDist;
//         ray.hitY = posY + rayDirY * perpWallDist;
//     }

//     return ray;
// }

RayCalc::Ray RayCalc::calcSingleRay(float rayAngle)
{
    Ray ray;
    ray.hitWall = false;
    ray.hitObject = nullptr;
    ray.distance = MAX_VIEW_DISTANCE;

    float posX = player.getX();
    float posY = player.getY();

    // Нормализуем угол
    rayAngle = fmod(rayAngle, 2 * M_PI);
    if (rayAngle < 0)
        rayAngle += 2 * M_PI;

    float rayDirX = cos(rayAngle);
    float rayDirY = sin(rayAngle);

    // Простой алгоритм - шагаем по лучу и проверяем стены
    for (float t = 0; t < MAX_VIEW_DISTANCE; t += 0.05f)
    {
        float testX = posX + rayDirX * t;
        float testY = posY + rayDirY * t;

        int mapX = static_cast<int>(testX);
        int mapY = static_cast<int>(testY);

        // Проверка выхода за границы карты
        if (mapX < 0 || mapX >= map.getWidth() || mapY < 0 || mapY >= map.getHeight())
        {
            ray.hitWall = true;
            ray.distance = t;
            ray.mapX = mapX;
            ray.mapY = mapY;
            ray.side = 0;
            break;
        }

        // Проверка стены
        if (map.getTitle(mapX, mapY) == '#')
        {
            ray.hitWall = true;
            ray.distance = t;
            ray.mapX = mapX;
            ray.mapY = mapY;
            ray.side = (abs(rayDirX) > abs(rayDirY)) ? 0 : 1;
            break;
        }
    }

    if (ray.hitWall)
    {
        std::cout << "Ray hit wall at (" << ray.mapX << "," << ray.mapY << ") distance: " << ray.distance << std::endl;
    }

    return ray;
}

const std::vector<RayCalc::Ray> &RayCalc::getRays() const
{
    return rays;
}

// ObjectVisibilityData RayCalc::getObjectVisibility(GameObject *obj)
// {
//     ObjectVisibilityData data;
//     data.isVisible = false;
//     data.distance = FLT_MAX;

//     for (const auto &ray : rays)
//     {
//         if (ray.hitObject == obj)
//         {
//             data.isVisible = true;
//             data.distance = std::min(data.distance, ray.distance);
//         }
//     }
//     return data;
// }

ObjectVisibilityData RayCalc::getObjectVisibility(GameObject *obj)
{
    ObjectVisibilityData data;
    data.isVisible = false;
    data.distance = FLT_MAX;

    // Простая проверка расстояния
    sf::Vector2f objPos = obj->getPosition();
    float dx = objPos.x - player.getX();
    float dy = objPos.y - player.getY();
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < 5.0f)
    {
        data.isVisible = true;
        data.distance = distance;
    }

    return data;
}