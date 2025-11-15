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
}

void Pseudo3DRenderer::renderWalls()
{

    const auto &rays = rayCalc.getRays();
    
    int renderedWalls = 0;
    int verticalWalls = 0;
    int horizontalWalls = 0;
    
    for (int i = 0; i < rays.size(); i++)
    {
        if (rays[i].hitWall && rays[i].distance < MAX_VIEW_DISTANCE)
        {
            renderWallSlice(i, rays[i]);
            renderedWalls++;
            
            if (rays[i].side == 0) {
                verticalWalls++;
            } else {
                horizontalWalls++;
            }
        }
    }
}

void Pseudo3DRenderer::renderWallSlice(int column, const RayCalc::Ray &ray)
{
    float wallHeight = calculateWallHeight(ray.distance);
    float brightness = calculateBrightness(ray.distance);

    if (wallHeight > window.getSize().y) {
        wallHeight = window.getSize().y;
    }
    
    sf::RectangleShape wallSlice(sf::Vector2f(1, wallHeight));
    float yPos = (window.getSize().y - wallHeight) / 2;
    wallSlice.setPosition(column, yPos);

    sf::Color wallColor = getWallColor(ray);

    //  // Применяем затемнение для расстояния
    wallColor.r = static_cast<sf::Uint8>(wallColor.r * brightness);
    wallColor.g = static_cast<sf::Uint8>(wallColor.g * brightness);
    wallColor.b = static_cast<sf::Uint8>(wallColor.b * brightness);

    // wallColor.r *= brightness;
    // wallColor.g *= brightness;
    // wallColor.b *= brightness;

    wallSlice.setFillColor(wallColor);

    window.draw(wallSlice);
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

float Pseudo3DRenderer::calculateWallHeight(float distance)
{
    if (distance <= 0) return 0;
    float height = window.getSize().y / distance;
    return height;
    //return std::min(height, float(window.getSize().y));
}

float Pseudo3DRenderer::calculateObjectScale(float distance)
{
    return 1.0f / (distance * 0.5f + 0.1f);
}

float Pseudo3DRenderer::calculateBrightness(float distance)
{
    return 1.0f / (1.0f + distance * 0.4f);
}

sf::Vector2f Pseudo3DRenderer::calculateScreenPosition(sf::Vector2f worldPos, float distance)
{
    // Конвертация мировых координат в экранные с учетом перспективы
    float screenX = (worldPos.x - player.getX()) * 100 / distance + window.getSize().x / 2;
    float screenY = (worldPos.y - player.getY()) * 100 / distance + window.getSize().y / 2;
    return sf::Vector2f(screenX, screenY);
}

sf::Color Pseudo3DRenderer::getWallColor(const RayCalc::Ray& ray)
{
    if (ray.side == 0) {
        return sf::Color(180, 80, 80);  // Тёмно-красный для вертикальных стен
    } else {
        return sf::Color(80, 80, 180);  // Тёмно-синий для горизонтальных стен
    }
}

float Pseudo3DRenderer::getDistanceToPlayer(GameObject *obj)
{
    sf::Vector2f objPos = obj->getPosition();
    float dx = objPos.x - player.getX();
    float dy = objPos.y - player.getY();
    return std::sqrt(dx * dx + dy * dy);
}