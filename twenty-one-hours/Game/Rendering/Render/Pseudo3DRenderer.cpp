#include "Pseudo3DRenderer.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

Pseudo3DRenderer::Pseudo3DRenderer(sf::RenderWindow &win, RayCalc &ray, Map &m, Player &p,
                                   const sf::Texture *enemyTex,
                                   const sf::Texture *bossTex)
    : window(win), rayCalc(ray), map(m), player(p), mSkyboxRenderer(win), mEnemyTexture(enemyTex), mBossTexture(bossTex)
{
    if (mSkyboxRenderer.loadFromFile("assets/gloomy_up.png"))
    {
        mSkyboxRenderer.setMode(SkyboxMode::Stretch);
        mSkyboxRenderer.setParallaxSpeed(0.0f);
    }

    // Инициализация текстур пола и потолка из атласа
    auto &textureManager = TextureManager::getInstance();
    const sf::Texture *atlas = textureManager.getAtlas("main");
    if (atlas)
    {
        mFloorRegion = textureManager.getRegion("SAND_FLOOR");
        mCeilingRegion = textureManager.getRegion("STONE_CEIL");

        sf::Image atlasImage = atlas->copyToImage();
        sf::Vector2u atlasSize = atlasImage.getSize();

        auto canExtractRegion = [&](const sf::IntRect &region) -> bool
        {
            if (region.width <= 0 || region.height <= 0)
                return false;
            return region.left >= 0 && region.top >= 0 &&
                   region.left + region.width <= static_cast<int>(atlasSize.x) &&
                   region.top + region.height <= static_cast<int>(atlasSize.y);
        };

        // Создаем отдельные текстуры для пола и потолка из атласа
        if (canExtractRegion(mFloorRegion))
        {
            sf::Image floorImage;
            floorImage.create(mFloorRegion.width, mFloorRegion.height);
            floorImage.copy(atlasImage, 0, 0, mFloorRegion, true);
            mFloorTexture.loadFromImage(floorImage);
            mFloorTexture.setRepeated(true);
        }

        if (canExtractRegion(mCeilingRegion))
        {
            sf::Image ceilingImage;
            ceilingImage.create(mCeilingRegion.width, mCeilingRegion.height);
            ceilingImage.copy(atlasImage, 0, 0, mCeilingRegion, true);
            mCeilingTexture.loadFromImage(ceilingImage);
            mCeilingTexture.setRepeated(true);
        }
    }
}

void Pseudo3DRenderer::render()
{
    window.clear(sf::Color::Black);
    int horizon = window.getSize().y / 2;
    mSkyboxRenderer.render(player.getAngle(), horizon);

    if (mCeilingEnabled)
    {
        renderCeiling();
    }
    renderFloor();
    renderWalls();
    // renderObjects();

    if (mDebugMode)
    {
        renderDebugInfo();
    }
}

void Pseudo3DRenderer::renderDebugInfo()
{
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        return;
    }

    sf::Text debugText;
    debugText.setFont(font);
    debugText.setCharacterSize(14);
    debugText.setFillColor(sf::Color::Green);
    debugText.setPosition(10, 10);

    std::stringstream ss;
    ss << "Player: (" << player.getX() << ", " << player.getY() << ")\n";
    ss << "Angle: " << player.getAngle() << "\n";
    ss << "FOV: " << player.fov << "\n";

    debugText.setString(ss.str());
    window.draw(debugText);
}

void Pseudo3DRenderer::renderFloor()
{
    int screenHeight = window.getSize().y;
    int screenWidth = window.getSize().x;
    int horizon = screenHeight / 2;

    if (screenHeight - horizon <= 0)
        return;

    // Создаем вертексный массив для пола
    sf::VertexArray floorVertices(sf::Quads, (screenHeight - horizon - 1) * 4);

    for (int y = horizon + 1; y < screenHeight; y++)
    {
        int idx = (y - horizon - 1) * 4;

        // Вычисляем мировые координаты для левого и правого края строки
        sf::Vector2f leftWorld = calculateWorldPosForFloorCeiling(0, y, true);
        sf::Vector2f rightWorld = calculateWorldPosForFloorCeiling(screenWidth - 1, y, true);
        sf::Vector2f leftWorldNext = calculateWorldPosForFloorCeiling(0, y + 1, true);
        sf::Vector2f rightWorldNext = calculateWorldPosForFloorCeiling(screenWidth - 1, y + 1, true);

        // Устанавливаем позиции вершин
        floorVertices[idx].position = sf::Vector2f(0, y);
        floorVertices[idx + 1].position = sf::Vector2f(screenWidth, y);
        floorVertices[idx + 2].position = sf::Vector2f(screenWidth, y + 1);
        floorVertices[idx + 3].position = sf::Vector2f(0, y + 1);

        // Масштаб текстуры пола (больше = более детально)
        float textureScale = 100.0f; // Увеличили для более детальных текстур

        // Устанавливаем текстурные координаты В ПИКСЕЛЯХ, а не нормализованные
        if (mFloorRegion.width > 0 && mFloorRegion.height > 0)
        {

            floorVertices[idx].texCoords = sf::Vector2f(
                leftWorld.x * textureScale,
                leftWorld.y * textureScale);
            floorVertices[idx + 1].texCoords = sf::Vector2f(
                rightWorld.x * textureScale,
                rightWorld.y * textureScale);
            floorVertices[idx + 2].texCoords = sf::Vector2f(
                rightWorldNext.x * textureScale,
                rightWorldNext.y * textureScale);
            floorVertices[idx + 3].texCoords = sf::Vector2f(
                leftWorldNext.x * textureScale,
                leftWorldNext.y * textureScale);
        }
        else
        {
            // Fallback: используем позицию на экране как текстурные координаты
            floorVertices[idx].texCoords = sf::Vector2f(0, y);
            floorVertices[idx + 1].texCoords = sf::Vector2f(screenWidth, y);
            floorVertices[idx + 2].texCoords = sf::Vector2f(screenWidth, y + 1);
            floorVertices[idx + 3].texCoords = sf::Vector2f(0, y + 1);
        }

        // Вычисляем затемнение в зависимости от расстояния
        float distanceLeft = sqrt((leftWorld.x - player.getX()) * (leftWorld.x - player.getX()) +
                                  (leftWorld.y - player.getY()) * (leftWorld.y - player.getY()));
        float distanceRight = sqrt((rightWorld.x - player.getX()) * (rightWorld.x - player.getX()) +
                                   (rightWorld.y - player.getY()) * (rightWorld.y - player.getY()));
        float avgDistance = (distanceLeft + distanceRight) / 2.0f;

        // Уменьшаем затемнение для пола, чтобы он был светлее
        float brightness = calculateBrightness(avgDistance);
        brightness = std::min(brightness, 0.8f); // Максимальная яркость 80%

        sf::Color color(255, 255, 255, 255);
        color.r = static_cast<sf::Uint8>(color.r * brightness);
        color.g = static_cast<sf::Uint8>(color.g * brightness);
        color.b = static_cast<sf::Uint8>(color.b * brightness);

        for (int i = 0; i < 4; i++)
        {
            floorVertices[idx + i].color = color;
        }
    }

    // Рисуем пол с текстурой
    sf::RenderStates floorStates;
    floorStates.texture = &mFloorTexture;
    // УБИРАЕМ эту строку: floorStates.texture->setRepeated(true);
    window.draw(floorVertices, floorStates);
}

void Pseudo3DRenderer::renderCeiling()
{
    int screenHeight = window.getSize().y;
    int screenWidth = window.getSize().x;
    int horizon = screenHeight / 2;

    if (horizon <= 0)
        return;

    // Создаем вертексный массив для потолка
    sf::VertexArray ceilingVertices(sf::Quads, horizon * 4);

    for (int y = 0; y < horizon; y++)
    {
        int idx = y * 4;

        // Вычисляем мировые координаты для левого и правого края строки
        sf::Vector2f leftWorld = calculateWorldPosForFloorCeiling(0, y, false);
        sf::Vector2f rightWorld = calculateWorldPosForFloorCeiling(screenWidth - 1, y, false);
        sf::Vector2f leftWorldNext = calculateWorldPosForFloorCeiling(0, y + 1, false);
        sf::Vector2f rightWorldNext = calculateWorldPosForFloorCeiling(screenWidth - 1, y + 1, false);

        // Устанавливаем позиции вершин
        ceilingVertices[idx].position = sf::Vector2f(0, y);
        ceilingVertices[idx + 1].position = sf::Vector2f(screenWidth, y);
        ceilingVertices[idx + 2].position = sf::Vector2f(screenWidth, y + 1);
        ceilingVertices[idx + 3].position = sf::Vector2f(0, y + 1);

        // Масштаб текстуры потолка
        float textureScale = 8.0f; // Увеличили для более детальных текстур

        // Устанавливаем текстурные координаты В ПИКСЕЛЯХ
        if (mCeilingRegion.width > 0 && mCeilingRegion.height > 0)
        {
            ceilingVertices[idx].texCoords = sf::Vector2f(
                leftWorld.x * textureScale,
                leftWorld.y * textureScale);
            ceilingVertices[idx + 1].texCoords = sf::Vector2f(
                rightWorld.x * textureScale,
                rightWorld.y * textureScale);
            ceilingVertices[idx + 2].texCoords = sf::Vector2f(
                rightWorldNext.x * textureScale,
                rightWorldNext.y * textureScale);
            ceilingVertices[idx + 3].texCoords = sf::Vector2f(
                leftWorldNext.x * textureScale,
                leftWorldNext.y * textureScale);
        }
        else
        {
            // Fallback: используем позицию на экране как текстурные координаты
            ceilingVertices[idx].texCoords = sf::Vector2f(0, y);
            ceilingVertices[idx + 1].texCoords = sf::Vector2f(screenWidth, y);
            ceilingVertices[idx + 2].texCoords = sf::Vector2f(screenWidth, y + 1);
            ceilingVertices[idx + 3].texCoords = sf::Vector2f(0, y + 1);
        }

        // Вычисляем затемнение в зависимости от расстояния
        float distanceLeft = sqrt((leftWorld.x - player.getX()) * (leftWorld.x - player.getX()) +
                                  (leftWorld.y - player.getY()) * (leftWorld.y - player.getY()));
        float distanceRight = sqrt((rightWorld.x - player.getX()) * (rightWorld.x - player.getX()) +
                                   (rightWorld.y - player.getY()) * (rightWorld.y - player.getY()));
        float avgDistance = (distanceLeft + distanceRight) / 2.0f;

        // Для потолка делаем затемнение сильнее
        float brightness = calculateBrightness(avgDistance * 1.15f);
        brightness = std::min(brightness, 0.7f); // Максимальная яркость 70%

        sf::Color color(255, 255, 255, 255);
        color.r = static_cast<sf::Uint8>(color.r * brightness);
        color.g = static_cast<sf::Uint8>(color.g * brightness);
        color.b = static_cast<sf::Uint8>(color.b * brightness);

        for (int i = 0; i < 4; i++)
        {
            ceilingVertices[idx + i].color = color;
        }
    }

    // Рисуем потолок с текстурой
    sf::RenderStates ceilingStates;
    ceilingStates.texture = &mCeilingTexture;
    // УБИРАЕМ эту строку: ceilingStates.texture->setRepeated(true);
    window.draw(ceilingVertices, ceilingStates);
}

// void Pseudo3DRenderer::renderFloor()
// {
//     int h = window.getSize().y;
//     int w = window.getSize().x;
//     int horizon = h / 2;

//     for (int y = horizon + 1; y < h; y++)
//     {
//         sf::VertexArray line(sf::Lines, 2);

//         float t = static_cast<float>(y - horizon) / (h - horizon);
//         sf::Color leftColor(100, 50, 50);
//         sf::Color rightColor(50, 50, 100);

//         line[0].position = sf::Vector2f(0, y);
//         line[0].color = leftColor;
//         line[1].position = sf::Vector2f(w, y);
//         line[1].color = rightColor;

//         window.draw(line);
//     }
// }

// void Pseudo3DRenderer::renderCeiling()
// {
//     int screenHeight = window.getSize().y;
//     int screenWidth = window.getSize().x;
//     int horizon = screenHeight / 2;

//     for (int y = 0; y < horizon; y++)
//     {
//         sf::VertexArray ceilingLine(sf::Lines, 2);

//         sf::Vector2f leftWorld = calculateWorldPosForFloorCeiling(0, y, false);
//         sf::Vector2f rightWorld = calculateWorldPosForFloorCeiling(screenWidth - 1, y, false);

//         sf::Color leftColor = getCeilingColor(leftWorld.x, leftWorld.y);
//         sf::Color rightColor = getCeilingColor(rightWorld.x, rightWorld.y);

//         float distance = static_cast<float>(horizon - y);
//         float brightness = 1.0f / (1.0f + distance * 0.1f);

//         leftColor.r = static_cast<sf::Uint8>(leftColor.r * brightness);
//         leftColor.g = static_cast<sf::Uint8>(leftColor.g * brightness);
//         leftColor.b = static_cast<sf::Uint8>(leftColor.b * brightness);

//         rightColor.r = static_cast<sf::Uint8>(rightColor.r * brightness);
//         rightColor.g = static_cast<sf::Uint8>(rightColor.g * brightness);
//         rightColor.b = static_cast<sf::Uint8>(rightColor.b * brightness);

//         ceilingLine[0].position = sf::Vector2f(0, y);
//         ceilingLine[0].color = leftColor;
//         ceilingLine[1].position = sf::Vector2f(screenWidth, y);
//         ceilingLine[1].color = rightColor;

//         window.draw(ceilingLine);
//     }
// }

void Pseudo3DRenderer::renderWalls()
{
    const auto &rays = rayCalc.getRays();

    for (int i = 0; i < rays.size(); i++)
    {
        if (rays[i].hitWall && rays[i].distance < getViewDistance())
        {
            renderWallSlice(i, rays[i]);
        }
    }
}

// void Pseudo3DRenderer::renderWallSlice(int column, const RayCalc::Ray &ray)
// {
//     if (!ray.hitSurface || ray.distance <= 0.01f)
//         return;

//     float wallHeight = calculateWallHeight(ray.distance);
//     float brightness = calculateBrightness(ray.distance);

//     sf::VertexArray wallQuad(sf::Quads, 4);
//     float x0 = static_cast<float>(column);
//     float x1 = x0 + 1.0f;
//     float y0 = (window.getSize().y - wallHeight) / 2.0f;
//     float y1 = y0 + wallHeight;

//     wallQuad[0].position = sf::Vector2f(x0, y0);
//     wallQuad[1].position = sf::Vector2f(x1, y0);
//     wallQuad[2].position = sf::Vector2f(x1, y1);
//     wallQuad[3].position = sf::Vector2f(x0, y1);

//     const sf::Texture *atlas = ray.hitSurface->getTextureAtlas();
//     sf::Color baseColor = ray.segmentColor;

//     if (atlas)
//     {
//         sf::IntRect region = ray.hitSurface->getTextureRegion();

//         float uOffset = (ray.segmentX / 3.0f) * region.width;
//         float vOffset = (ray.segmentY / 3.0f) * region.height;
//         float uScale = region.width / 3.0f;
//         float vScale = region.height / 3.0f;

//         wallQuad[0].texCoords = sf::Vector2f((region.left + uOffset), (region.top + vOffset));
//         wallQuad[1].texCoords = sf::Vector2f((region.left + uOffset + uScale), (region.top + vOffset));
//         wallQuad[2].texCoords = sf::Vector2f((region.left + uOffset + uScale), (region.top + vOffset + vScale));
//         wallQuad[3].texCoords = sf::Vector2f((region.left + uOffset), (region.top + vOffset + vScale));

//         sf::Color dimmed(255, 255, 255, 255);
//         dimmed.r = static_cast<sf::Uint8>(255 * brightness);
//         dimmed.g = static_cast<sf::Uint8>(255 * brightness);
//         dimmed.b = static_cast<sf::Uint8>(255 * brightness);

//         for (int i = 0; i < 4; i++)
//         {
//             wallQuad[i].color = dimmed;
//         }
//     }
//     else
//     {
//         sf::Color color = baseColor;
//         color.r = static_cast<sf::Uint8>(color.r * brightness);
//         color.g = static_cast<sf::Uint8>(color.g * brightness);
//         color.b = static_cast<sf::Uint8>(color.b * brightness);

//         for (int i = 0; i < 4; i++)
//         {
//             wallQuad[i].color = color;
//         }
//     }

//     sf::RenderStates states;
//     states.texture = atlas;
//     window.draw(wallQuad, states);
// }

void Pseudo3DRenderer::renderWallSlice(int column, const RayCalc::Ray &ray)
{
    if (!ray.hitSurface || ray.distance <= 0.0f)
        return;

    float wallHeight = calculateWallHeight(ray.distance);
    float brightness = calculateBrightness(ray.distance);

    sf::VertexArray wallQuad(sf::Quads, 4);
    float x0 = static_cast<float>(column);
    float x1 = x0 + 1.0f;
    float y0 = (window.getSize().y - wallHeight) / 2.0f;
    float y1 = y0 + wallHeight;

    wallQuad[0].position = sf::Vector2f(x0, y0);
    wallQuad[1].position = sf::Vector2f(x1, y0);
    wallQuad[2].position = sf::Vector2f(x1, y1);
    wallQuad[3].position = sf::Vector2f(x0, y1);

    const sf::Texture *atlas = ray.hitSurface->getTextureAtlas();
    sf::Color baseColor = ray.segmentColor;

    if (atlas)
    {
        // Используем старый метод текстурирования (без сегментов)
        float wallX = 0.0f;
        if (ray.side == 0) // Вертикальная сторона
            wallX = ray.hitY - floor(ray.hitY);
        else // Горизонтальная сторона
            wallX = ray.hitX - floor(ray.hitX);

        sf::IntRect region = ray.hitSurface->getTextureRegion();

        // Вычисляем координату текстуры по X
        float texX = region.left + wallX * region.width;
        float texXEnd = region.left + wallX * region.width + 1;

        // Координаты по Y (используем всю высоту текстуры)
        float texYStart = region.top;
        float texYEnd = region.top + region.height;

        wallQuad[0].texCoords = sf::Vector2f(texX, texYStart);
        wallQuad[1].texCoords = sf::Vector2f(texXEnd, texYStart);
        wallQuad[2].texCoords = sf::Vector2f(texXEnd, texYEnd);
        wallQuad[3].texCoords = sf::Vector2f(texX, texYEnd);

        sf::Color dimmed(255, 255, 255);
        dimmed.r = static_cast<sf::Uint8>(255 * brightness);
        dimmed.g = static_cast<sf::Uint8>(255 * brightness);
        dimmed.b = static_cast<sf::Uint8>(255 * brightness);

        for (int i = 0; i < 4; i++)
        {
            wallQuad[i].color = dimmed;
        }
    }
    else
    {
        // Fallback: цвет стены
        sf::Color color = baseColor;
        color.r = static_cast<sf::Uint8>(color.r * brightness);
        color.g = static_cast<sf::Uint8>(color.g * brightness);
        color.b = static_cast<sf::Uint8>(color.b * brightness);

        for (int i = 0; i < 4; i++)
        {
            wallQuad[i].color = color;
        }
    }

    sf::RenderStates states;
    states.texture = atlas;
    window.draw(wallQuad, states);
}

sf::Vector2f Pseudo3DRenderer::calculateWorldPosForFloorCeiling(int screenX, int screenY, bool isFloor) const
{
    int screenHeight = window.getSize().y;
    int screenWidth = window.getSize().x;

    float playerX = player.getX();
    float playerY = player.getY();
    float playerAngle = player.getAngle();

    float distanceToProjection = screenWidth / (2.0f * tan(player.fov / 2.0f));

    float cameraHeight = 0.5f;

    float relativeY;
    if (isFloor)
    {
        relativeY = static_cast<float>(screenY - screenHeight / 2);
    }
    else
    {
        relativeY = static_cast<float>(screenHeight / 2 - screenY);
    }

    float distance = (cameraHeight * distanceToProjection) / relativeY;

    float rayAngle = playerAngle - (player.fov / 2.0f) +
                     (player.fov * screenX / static_cast<float>(screenWidth));

    float worldX = playerX + distance * cos(rayAngle);
    float worldY = playerY + distance * sin(rayAngle);

    return sf::Vector2f(worldX, worldY);
}

sf::Color Pseudo3DRenderer::getFloorColor(float worldX, float worldY) const
{
    auto floor = map.getFloorAt(worldX, worldY);
    if (floor)
    {
        return floor->getAverageColor();
    }
    return sf::Color(210, 180, 140);
}

sf::Color Pseudo3DRenderer::getCeilingColor(float worldX, float worldY) const
{
    auto ceiling = map.getCeilingAt(worldX, worldY);
    if (ceiling)
    {
        return ceiling->getAverageColor();
    }
    return sf::Color(100, 100, 100);
}

void Pseudo3DRenderer::renderObject(GameObject *obj, const ObjectVisibilityData &data)
{
    //
}

float Pseudo3DRenderer::calculateWallHeight(float distance) const
{
    if (distance <= 0)
        return 0;
    float height = window.getSize().y / distance;
    return height;
}

float Pseudo3DRenderer::calculateObjectScale(float distance)
{
    return 1.0f / (distance * 0.5f + 0.1f);
}

float Pseudo3DRenderer::calculateBrightness(float distance) const
{
    float viewDistance = getViewDistance();

    // Нормализация расстояния в диапазон [0, 1]
    float normalized = distance / (viewDistance > 1.0f ? viewDistance : 1.0f);

    // Ограничиваем нормализованное значение вручную (без std::clamp)
    if (normalized < 0.0f)
        normalized = 0.0f;
    else if (normalized > 1.0f)
        normalized = 1.0f;

    // Глобальная карта становится темнее вдали, чтобы скрыть дальнюю прорисовку
    float fog = 1.0f - normalized;

    // Вычисляем яркость
    float brightness = 0.2f + fog * fog * 0.8f;

    // Ограничиваем яркость вручную (без std::clamp)
    if (brightness < 0.2f)
        brightness = 0.2f;
    else if (brightness > 1.0f)
        brightness = 1.0f;

    return brightness;
}

float Pseudo3DRenderer::getViewDistance() const
{
    if (map.getLevelType() == Map::LevelType::OpenWorld)
    {
        return 14.0f;
    }
    return DEFAULT_VIEW_DISTANCE;
}

sf::Vector2f Pseudo3DRenderer::calculateScreenPosition(sf::Vector2f worldPos, float distance)
{
    float screenX = (worldPos.x - player.getX()) * 100 / distance + window.getSize().x / 2;
    float screenY = (worldPos.y - player.getY()) * 100 / distance + window.getSize().y / 2;
    return sf::Vector2f(screenX, screenY);
}

sf::Color Pseudo3DRenderer::getWallColor(const RayCalc::Ray &ray)
{
    if (ray.side == 0)
    {
        return sf::Color(180, 80, 80);
    }
    else
    {
        return sf::Color(80, 80, 180);
    }
}

float Pseudo3DRenderer::getDistanceToPlayer(GameObject *obj)
{
    sf::Vector2f objPos = obj->getPosition();
    float dx = objPos.x - player.getX();
    float dy = objPos.y - player.getY();
    return std::sqrt(dx * dx + dy * dy);
}

// void Pseudo3DRenderer::renderSprite(const sf::Vector2f& spritePos,
//                                     const sf::Vector2f& playerPos,
//                                     const std::string& textureName,
//                                     float rotation) {
//     // Вычисляем расстояние до спрайта
//     float dx = spritePos.x - playerPos.x;
//     float dy = spritePos.y - playerPos.y;
//     float distance = std::sqrt(dx*dx + dy*dy);

//     if (distance > MAX_VIEW_DISTANCE || distance < 0.1f)
//         return;

//     // Вычисляем экранные координаты
//     float angleToSprite = std::atan2(dy, dx);
//     float playerAngle = player.getAngle();

//     // Нормализуем разность углов
//     float angleDiff = angleToSprite - playerAngle;
//     if (angleDiff > M_PI) angleDiff -= 2 * M_PI;
//     if (angleDiff < -M_PI) angleDiff += 2 * M_PI;

//     // Если спрайт в пределах FOV
//     if (std::abs(angleDiff) < player.fov / 2.0f) {
//         // Простая отрисовка круга вместо спрайта (временно)
//         float screenX = (angleDiff / (player.fov / 2.0f) + 1.0f) * 0.5f * window.getSize().x;
//         float screenY = window.getSize().y / 2.0f;

//         float scale = 100.0f / distance; // Масштаб в зависимости от расстояния
//         scale = (scale < 5.0f) ? 5.0f : (scale > 50.0f ? 50.0f : scale);

//         sf::CircleShape enemyShape(scale);
//         enemyShape.setFillColor(sf::Color::Red);
//         enemyShape.setPosition(screenX - scale, screenY - scale);
//         window.draw(enemyShape);
//     }
// }

void Pseudo3DRenderer::renderSprite(const sf::Vector2f &spritePos,
                                    const sf::Vector2f &playerPos,
                                    const sf::Texture *texture,
                                    const sf::IntRect &region,
                                    float rotation,
                                    float visibility,
                                    float worldHeight,
                                    const sf::Color &tint)
{
    if (!texture || visibility <= 0.01f)
        return;

    float dx = spritePos.x - playerPos.x;
    float dy = spritePos.y - playerPos.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Минимальное расстояние - не слишком маленькое
    if (distance > 20.0f || distance < 0.3f)
        return;

    // Угол и проверка FOV
    float angleToSprite = std::atan2(dy, dx);
    float angleDiff = angleToSprite - player.getAngle();
    while (angleDiff > M_PI)
        angleDiff -= 2 * M_PI;
    while (angleDiff < -M_PI)
        angleDiff += 2 * M_PI;

    if (angleDiff > player.fov / 2.0f || angleDiff < -player.fov / 2.0f)
        return;

    // X координата
    float screenX = (angleDiff / (player.fov / 2.0f) + 1.0f) * 0.5f * window.getSize().x;

    float screenHeight = static_cast<float>(window.getSize().y);
    float screenWidth = static_cast<float>(window.getSize().x);
    float horizon = screenHeight / 2.0f;
    float distanceToProjection = screenWidth / (2.0f * tan(player.fov / 2.0f));

    float cameraHeight = 0.5f;

    float spriteScreenHeight = (worldHeight * distanceToProjection) / distance;
    float screenY = horizon + (cameraHeight * distanceToProjection) / distance;

    float minHeight = 24.0f;
    if (spriteScreenHeight < minHeight)
        spriteScreenHeight = minHeight;

    // ===== ОТРИСОВКА =====
    sf::Sprite sprite(*texture);
    sprite.setTextureRect(region);
    sprite.setOrigin(region.width / 2.0f, region.height);
    sprite.setPosition(screenX, screenY);

    float spriteScreenWidth = spriteScreenHeight * (region.width / static_cast<float>(region.height));
    sprite.setScale(spriteScreenWidth / region.width, spriteScreenHeight / region.height);

    // Яркость от расстояния, прозрачность от видимости
    float brightness = 1.0f / (1.0f + distance * 0.1f);
    if (brightness > 1.0f)
        brightness = 1.0f;
    if (brightness < 0.4f)
        brightness = 0.4f;

    sf::Color color = tint;
    color.r = static_cast<sf::Uint8>(color.r * brightness);
    color.g = static_cast<sf::Uint8>(color.g * brightness);
    color.b = static_cast<sf::Uint8>(color.b * brightness);
    color.a = static_cast<sf::Uint8>(color.a * visibility);

    sprite.setColor(color);

    window.draw(sprite);
}