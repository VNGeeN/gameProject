#include "Pseudo3DRenderer.h"
#include <iostream>

Pseudo3DRenderer::Pseudo3DRenderer(sf::RenderWindow &win, RayCalc &ray, Map &m, Player &p)
    : window(win), rayCalc(ray), map(m), player(p)
{
}

void Pseudo3DRenderer::render()
{
    window.clear(sf::Color::Black);
    renderWalls();
    // renderObjects();
    // renderHUD();
}

// void Pseudo3DRenderer::renderWalls()
// {
//     const auto &rays = rayCalc.getRays();

//     // std::cout << "Rendering " << rays.size() << " wall slices" << std::endl;

//     for (int i = 0; i < rays.size(); i++)
//     {
//         if (rays[i].hitWall)
//         {
//             renderWallSlice(i, rays[i]);
//         }
//     }
// }

void Pseudo3DRenderer::renderWalls()
{
    const auto &rays = rayCalc.getRays();
    
    int renderedWalls = 0;
    for (int i = 0; i < rays.size(); i++)
    {
        if (rays[i].hitWall && rays[i].distance < 20.0f)
        {
            renderWallSlice(i, rays[i]);
            renderedWalls++;
        }
    }
    
    std::cout << "Rendered " << renderedWalls << " wall slices" << std::endl;
}

// void Pseudo3DRenderer::renderWallSlice(int column, const RayCalc::Ray &ray)
// {
//     float wallHeight = calculateWallHeight(ray.distance);
//     float brightness = calculateBrightness(ray.distance);

//     std::cout << "Wall slice at column " << column << ", height " << wallHeight << ", distance " << ray.distance << std::endl;

//     sf::RectangleShape wallSlice(sf::Vector2f(1, wallHeight));
//     wallSlice.setPosition(column, (window.getSize().y - wallHeight) / 2);

//     // sf::Color wallColor = getWallColor(ray.hitX, ray.hitY);
//     sf::Color wallColor = getWallColor(ray);
//     wallColor.r *= brightness;
//     wallColor.g *= brightness;
//     wallColor.b *= brightness;
//     wallSlice.setFillColor(wallColor);

//     window.draw(wallSlice);
// }

void Pseudo3DRenderer::renderWallSlice(int column, const RayCalc::Ray &ray)
{
    float wallHeight = calculateWallHeight(ray.distance);
    float brightness = calculateBrightness(ray.distance);

    // Убедимся, что высота стены разумная
    if (wallHeight > 1000) wallHeight = 1000;
    
    sf::RectangleShape wallSlice(sf::Vector2f(1, wallHeight));
    float yPos = (window.getSize().y - wallHeight) / 2;
    wallSlice.setPosition(column, yPos);

    sf::Color wallColor = getWallColor(ray);
    wallColor.r *= brightness;
    wallColor.g *= brightness;
    wallColor.b *= brightness;
    wallSlice.setFillColor(wallColor);

    window.draw(wallSlice);
    
    std::cout << "Wall slice at column " << column << ", height " << wallHeight << ", distance " << ray.distance << std::endl;
}

void Pseudo3DRenderer::renderObjects()
{
    // auto objects = map.getObjects();

    // // Сортируем объекты по расстоянию (от дальних к ближним)
    // std::sort(objects.begin(), objects.end(),
    //           [this](GameObject *a, GameObject *b)
    //           {
    //               return getDistanceToPlayer(a) > getDistanceToPlayer(b);
    //           });

    // for (auto *obj : objects)
    // {
    //     auto visibility = rayCalc.getObjectVisibility(obj);
    //     if (visibility.isVisible)
    //     {
    //         renderObject(obj, visibility);
    //     }
    // }
}

void Pseudo3DRenderer::renderObject(GameObject *obj, const ObjectVisibilityData &data)
{
    // obj->updateFromRayData(data);
    // const auto &sprite = obj->getSprite();

    // sf::Vector2f screenPos = calculateScreenPosition(obj->getPosition(), data.distance);
    // float scale = calculateObjectScale(data.distance);

    // sf::Sprite renderSprite = sprite;
    // renderSprite.setScale(scale, scale);
    // renderSprite.setPosition(screenPos);

    // sf::Color color = renderSprite.getColor();
    // float brightness = calculateBrightness(data.distance);
    // color.r *= brightness;
    // color.g *= brightness;
    // color.b *= brightness;
    // renderSprite.setColor(color);

    // window.draw(renderSprite);
}

// float Pseudo3DRenderer::calculateWallHeight(float distance)
// {
//     // Простая перспектива: чем дальше, тем меньше стена
//     return window.getSize().y / (distance + 0.1f);
// }

float Pseudo3DRenderer::calculateWallHeight(float distance)
{
    if (distance <= 0) return 0;
    float height = window.getSize().y / distance;
    return std::min(height, float(window.getSize().y));
}

float Pseudo3DRenderer::calculateObjectScale(float distance)
{
    return 1.0f / (distance * 0.5f + 0.1f);
}

float Pseudo3DRenderer::calculateBrightness(float distance)
{
    return 1.0f / (1.0f + distance * 0.3f);
}

sf::Vector2f Pseudo3DRenderer::calculateScreenPosition(sf::Vector2f worldPos, float distance)
{
    // Конвертация мировых координат в экранные с учетом перспективы
    // Это упрощенная версия - в реальности нужны тригонометрические расчеты
    float screenX = (worldPos.x - player.getX()) * 100 / distance + window.getSize().x / 2;
    float screenY = (worldPos.y - player.getY()) * 100 / distance + window.getSize().y / 2;
    return sf::Vector2f(screenX, screenY);
}

// sf::Color Pseudo3DRenderer::getWallColor(float hitX, float hitY)
// {
//     // Простая реализация - возвращаем фиксированный цвет
//     // В проекте можно выбирать цвет на основе типа стены или текстуры
//     return sf::Color(150, 150, 150); // Серый цвет
// }

sf::Color Pseudo3DRenderer::getWallColor(const RayCalc::Ray& ray)
{
    // Разные цвета для отладки
    if (ray.side == 0) {
        return sf::Color(255, 0, 0); // Красный
    } else {
        return sf::Color(0, 0, 255); // Синий
    }
}

float Pseudo3DRenderer::getDistanceToPlayer(GameObject *obj)
{
    sf::Vector2f objPos = obj->getPosition();
    float dx = objPos.x - player.getX();
    float dy = objPos.y - player.getY();
    return std::sqrt(dx * dx + dy * dy);
}