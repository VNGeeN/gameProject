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
//         float cameraX = 2 * i / float(rayCount) - 1; // от -1 до 1
//         float rayDirX = cos(player.getAngle()) + (-sin(player.getAngle())) * player.fov * cameraX;
//         float rayDirY = sin(player.getAngle()) + (cos(player.getAngle())) * player.fov * cameraX;

//         float angle = atan2(rayDirY, rayDirX);
//         Ray ray = calcSingleRay(angle);
//         rays.push_back(ray);
//     }

// }

void RayCalc::calcRays(int rayCount)
{
    rays.clear();

    float posX = player.getX();
    float posY = player.getY();
    float angle = player.getAngle();

    for (int i = 0; i < rayCount; i++)
    {
        // Вычисляем угол для текущего луча
        float rayAngle = angle - (player.fov / 2.0f) + (player.fov * i / float(rayCount));

        Ray ray = calcSingleRay(rayAngle);
        rays.push_back(ray);
    }
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
    ray.wallX = 0.0f;

    float posX = player.getX();
    float posY = player.getY();

    // Нормализуем угол
    rayAngle = fmod(rayAngle, 2 * M_PI);
    if (rayAngle < 0)
    {
        rayAngle += 2 * M_PI;
    }

    float rayDirX = cos(rayAngle);
    float rayDirY = sin(rayAngle);

    // Текущая клетка карты
    int mapX = static_cast<int>(posX);
    int mapY = static_cast<int>(posY);

    // Длина луча от одной стороны до другой в направлении луча
    float deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);

    float sideDistX, sideDistY;
    int stepX, stepY;
    int side;

    // Вычисляем step и initial sideDist
    if (rayDirX < 0)
    {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
    }

    if (rayDirY < 0)
    {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
    }

    // DDA алгоритм
    bool hit = false;
    int steps = 0;
    const int MAX_STEPS = 100; // Защита от бесконечного цикла

    while (!hit && steps < MAX_STEPS)
    {
        steps++;

        // Переход к следующей клетке
        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0; // Вертикальная сторона
        }
        else
        {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1; // Горизонтальная сторона
        }

        // Проверка, не вышли ли за пределы карты
        if (mapX < 0 || mapX >= map.getWidth() || mapY < 0 || mapY >= map.getHeight())
        {
            hit = true;
            ray.hitWall = true;
            ray.distance = MAX_VIEW_DISTANCE;
            break;
        }

        // Проверка, является ли клетка стеной
        if (map.getTitle(mapX, mapY) == '#')
        {
            hit = true;
            ray.hitWall = true;

            // Вычисляем перпендикулярное расстояние до стены
            if (side == 0)
            {
                ray.distance = (mapX - posX + (1 - stepX) / 2) / rayDirX;
            }
            else
            {
                ray.distance = (mapY - posY + (1 - stepY) / 2) / rayDirY;
            }

            // Убедимся, что расстояние положительное
            if (ray.distance < 0)
            {
                ray.distance = 0.1f;
            }

            // Вычисляем точку попадания на стене для текстурных координат
            float wallX;
            if (side == 0) {
                wallX = posY + ray.distance * rayDirY;
            } else {
                wallX = posX + ray.distance * rayDirX;
            }
            wallX -= floor(wallX);
            
            ray.wallX = wallX;

            ray.mapX = mapX;
            ray.mapY = mapY;
            ray.side = side;
            ray.hitX = posX + rayDirX * ray.distance;
            ray.hitY = posY + rayDirY * ray.distance;
        }
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