#include "SegmentedWall.h"
#include <iostream>
#include <cmath>

SegmentedWall::SegmentedWall(const sf::Vector2f &position, const sf::Vector2f &size)
    : mPosition(position), mSize(size)
{
    initializeSegments();
    updateGlobalBounds();
}

void SegmentedWall::initializeSegments()
{
    const float segmentWidth = mSize.x / 3.0f;
    const float segmentHeight = mSize.y / 3.0f;

    sf::Color cornerColor(255, 100, 100); 
    sf::Color edgeColor(100, 255, 100);   
    sf::Color centerColor(100, 100, 255); 

    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            auto &segment = mSegments[y][x];

            float segX = mPosition.x + x * segmentWidth;
            float segY = mPosition.y + y * segmentHeight;

            segment.vertices[0].position = sf::Vector2f(segX, segY);
            segment.vertices[1].position = sf::Vector2f(segX + segmentWidth, segY);
            segment.vertices[2].position = sf::Vector2f(segX + segmentWidth, segY + segmentHeight);
            segment.vertices[3].position = sf::Vector2f(segX, segY + segmentHeight);

            segment.hitbox = sf::FloatRect(segX, segY, segmentWidth, segmentHeight);

            if ((x == 0 || x == 2) && (y == 0 || y == 2))
            {
                segment.color = cornerColor; 
            }
            else if (x == 1 && y == 1)
            {
                segment.color = centerColor; 
            }
            else
            {
                segment.color = edgeColor; 
            }

            for (int i = 0; i < 4; i++)
            {
                segment.vertices[i].color = segment.color;
            }

            segment.isPassable = false;
        }
    }
}

void SegmentedWall::draw(sf::RenderTarget &target) const
{
    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            const auto &segment = mSegments[y][x];

            if (!segment.isPassable || mDebugMode)
            {
                target.draw(segment.vertices);

                if (mDebugMode)
                {
                    sf::RectangleShape debugRect(sf::Vector2f(
                        segment.hitbox.width,
                        segment.hitbox.height));
                    debugRect.setPosition(segment.hitbox.left, segment.hitbox.top);
                    debugRect.setFillColor(sf::Color::Transparent);

                    debugRect.setOutlineColor(segment.isPassable ? sf::Color::Green : sf::Color::Red);
                    debugRect.setOutlineThickness(0.03f);
                    target.draw(debugRect);
                }
            }
        }
    }
}

SegmentedWall::Segment *SegmentedWall::getSegmentAt(float localX, float localY)
{
    if (localX < 0 || localX >= 1.0f || localY < 0 || localY >= 1.0f)
    {
        return nullptr;
    }

    int segX = static_cast<int>(localX * 3);
    int segY = static_cast<int>(localY * 3);

    segX = std::max(0, std::min(2, segX));
    segY = std::max(0, std::min(2, segY));

    return &mSegments[segY][segX];
}

const SegmentedWall::Segment *SegmentedWall::getSegmentAt(float localX, float localY) const
{
    if (localX < 0 || localX >= 1.0f || localY < 0 || localY >= 1.0f)
    {
        return nullptr;
    }

    int segX = static_cast<int>(localX * 3);
    int segY = static_cast<int>(localY * 3);

    segX = std::max(0, std::min(2, segX));
    segY = std::max(0, std::min(2, segY));

    return &mSegments[segY][segX];
}

bool SegmentedWall::checkRayCollision(const sf::Vector2f &rayStart, const sf::Vector2f &rayDir,
                                      float &distance, int &segmentX, int &segmentY) const
{
    bool hit = false;
    float minDistance = std::numeric_limits<float>::max();
    int hitSegX = -1, hitSegY = -1;

    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            const auto &segment = mSegments[y][x];

            if (segment.isPassable)
                continue;

            float segDistance;
            if (checkSegmentRayCollision(segment, rayStart, rayDir, segDistance))
            {
                if (segDistance < minDistance)
                {
                    minDistance = segDistance;
                    hitSegX = x;
                    hitSegY = y;
                    hit = true;
                }
            }
        }
    }

    if (hit)
    {
        distance = minDistance;
        segmentX = hitSegX;
        segmentY = hitSegY;
    }

    return hit;
}

bool SegmentedWall::checkSegmentRayCollision(const Segment &segment, const sf::Vector2f &rayStart,
                                             const sf::Vector2f &rayDir, float &distance) const
{
    const sf::FloatRect &bounds = segment.hitbox;

    float t1 = (bounds.left - rayStart.x) / rayDir.x;
    float t2 = (bounds.left + bounds.width - rayStart.x) / rayDir.x;
    float t3 = (bounds.top - rayStart.y) / rayDir.y;
    float t4 = (bounds.top + bounds.height - rayStart.y) / rayDir.y;

    float tmin = std::max(std::min(t1, t2), std::min(t3, t4));
    float tmax = std::min(std::max(t1, t2), std::max(t3, t4));

    if (tmax < 0 || tmin > tmax)
    {
        return false;
    }

    distance = tmin;
    return true;
}

void SegmentedWall::setSegmentColor(int x, int y, const sf::Color &color)
{
    if (x >= 0 && x < 3 && y >= 0 && y < 3)
    {
        auto &segment = mSegments[y][x];
        segment.color = color;

        for (int i = 0; i < 4; i++)
        {
            segment.vertices[i].color = color;
        }
    }
}

void SegmentedWall::setAllSegmentsColor(const sf::Color &color)
{
    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            setSegmentColor(x, y, color);
        }
    }
}

sf::Color SegmentedWall::getSegmentColor(int x, int y) const
{
    if (x >= 0 && x < 3 && y >= 0 && y < 3)
    {
        return mSegments[y][x].color;
    }
    return sf::Color::Black;
}

void SegmentedWall::setSegmentPassable(int x, int y, bool passable)
{
    if (x >= 0 && x < 3 && y >= 0 && y < 3)
    {
        mSegments[y][x].isPassable = passable;
    }
}

bool SegmentedWall::isSegmentPassable(int x, int y) const
{
    if (x >= 0 && x < 3 && y >= 0 && y < 3)
    {
        return mSegments[y][x].isPassable;
    }
    return true; 
}

void SegmentedWall::setSize(const sf::Vector2f &size)
{
    mSize = size;
    initializeSegments(); 
    updateGlobalBounds();
}

void SegmentedWall::updateSegmentVertices(int x, int y)
{
    if (x < 0 || x >= 3 || y < 0 || y >= 3)
        return;

    const float segmentWidth = mSize.x / 3.0f;
    const float segmentHeight = mSize.y / 3.0f;

    auto &segment = mSegments[y][x];
    float segX = mPosition.x + x * segmentWidth;
    float segY = mPosition.y + y * segmentHeight;

    segment.vertices[0].position = sf::Vector2f(segX, segY);
    segment.vertices[1].position = sf::Vector2f(segX + segmentWidth, segY);
    segment.vertices[2].position = sf::Vector2f(segX + segmentWidth, segY + segmentHeight);
    segment.vertices[3].position = sf::Vector2f(segX, segY + segmentHeight);

    segment.hitbox = sf::FloatRect(segX, segY, segmentWidth, segmentHeight);
}

void SegmentedWall::updateGlobalBounds()
{
    mGlobalBounds = sf::FloatRect(mPosition.x, mPosition.y, mSize.x, mSize.y);
}