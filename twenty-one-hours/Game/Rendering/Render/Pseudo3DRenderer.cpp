#include "Pseudo3DRenderer.h"
#include <iostream>
#include <sstream>

Pseudo3DRenderer::Pseudo3DRenderer(sf::RenderWindow &win, RayCalc &ray, Map &m, Player &p)
    : window(win), rayCalc(ray), map(m), player(p), mSkyboxRenderer(win)
{
    if (mSkyboxRenderer.loadFromFile("assets/gloomy_up.png"))
    {
        mSkyboxRenderer.setMode(SkyboxMode::Stretch); 
        mSkyboxRenderer.setParallaxSpeed(0.0f);       
    }
}

void Pseudo3DRenderer::render()
{
    window.clear(sf::Color::Black);
    int horizon = window.getSize().y / 2;
    mSkyboxRenderer.render(player.getAngle(), horizon);

    renderCeiling(); 
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
    int h = window.getSize().y;
    int w = window.getSize().x;
    int horizon = h / 2;

    for (int y = horizon + 1; y < h; y++)
    {
        sf::VertexArray line(sf::Lines, 2);

        float t = static_cast<float>(y - horizon) / (h - horizon);
        sf::Color leftColor(100, 50, 50);  
        sf::Color rightColor(50, 50, 100); 

        line[0].position = sf::Vector2f(0, y);
        line[0].color = leftColor;
        line[1].position = sf::Vector2f(w, y);
        line[1].color = rightColor;

        window.draw(line);
    }
}

void Pseudo3DRenderer::renderCeiling()
{
    int screenHeight = window.getSize().y;
    int screenWidth = window.getSize().x;
    int horizon = screenHeight / 2;

    for (int y = 0; y < horizon; y++)
    {
        sf::VertexArray ceilingLine(sf::Lines, 2);

        sf::Vector2f leftWorld = calculateWorldPosForFloorCeiling(0, y, false);
        sf::Vector2f rightWorld = calculateWorldPosForFloorCeiling(screenWidth - 1, y, false);

        sf::Color leftColor = getCeilingColor(leftWorld.x, leftWorld.y);
        sf::Color rightColor = getCeilingColor(rightWorld.x, rightWorld.y);

        float distance = static_cast<float>(horizon - y);
        float brightness = 1.0f / (1.0f + distance * 0.1f);

        leftColor.r = static_cast<sf::Uint8>(leftColor.r * brightness);
        leftColor.g = static_cast<sf::Uint8>(leftColor.g * brightness);
        leftColor.b = static_cast<sf::Uint8>(leftColor.b * brightness);

        rightColor.r = static_cast<sf::Uint8>(rightColor.r * brightness);
        rightColor.g = static_cast<sf::Uint8>(rightColor.g * brightness);
        rightColor.b = static_cast<sf::Uint8>(rightColor.b * brightness);

        ceilingLine[0].position = sf::Vector2f(0, y);
        ceilingLine[0].color = leftColor;
        ceilingLine[1].position = sf::Vector2f(screenWidth, y);
        ceilingLine[1].color = rightColor;

        window.draw(ceilingLine);
    }
}

void Pseudo3DRenderer::renderWalls()
{
    const auto &rays = rayCalc.getRays();

    for (int i = 0; i < rays.size(); i++)
    {
        if (rays[i].hitWall && rays[i].distance < MAX_VIEW_DISTANCE)
        {
            renderWallSlice(i, rays[i]);
        }
    }
}

void Pseudo3DRenderer::renderWallSlice(int column, const RayCalc::Ray &ray)
{
    if (!ray.hitSurface || ray.distance <= 0.01f)
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
        sf::IntRect region = ray.hitSurface->getTextureRegion();

        float uOffset = (ray.segmentX / 3.0f) * region.width;
        float vOffset = (ray.segmentY / 3.0f) * region.height;
        float uScale = region.width / 3.0f;
        float vScale = region.height / 3.0f;

        wallQuad[0].texCoords = sf::Vector2f(region.left + uOffset, region.top + vOffset);
        wallQuad[1].texCoords = sf::Vector2f(region.left + uOffset + uScale, region.top + vOffset);
        wallQuad[2].texCoords = sf::Vector2f(region.left + uOffset + uScale, region.top + vOffset + vScale);
        wallQuad[3].texCoords = sf::Vector2f(region.left + uOffset, region.top + vOffset + vScale);

        sf::Color dimmed(255, 255, 255, 255);
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
    return 1.0f / (1.0f + distance * 0.4f);
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