#include "Surface.h"
#include <cmath>
#include <algorithm>

const std::array<sf::Color, 5> Surface::MATERIAL_BASE_COLORS = {{
    sf::Color(210, 180, 140), 
    sf::Color(100, 100, 100), 
    sf::Color(120, 120, 120), 
    sf::Color(139, 69, 19),   
    sf::Color(34, 139, 34)    
}};

const std::array<sf::Color, 9> Surface::SEGMENT_COLORS = {{
    sf::Color(255, 100, 100), 
    sf::Color(255, 100, 100),
    sf::Color(255, 100, 100),
    sf::Color(255, 100, 100),

    sf::Color(100, 255, 100), 
    sf::Color(100, 255, 100),
    sf::Color(100, 255, 100),
    sf::Color(100, 255, 100),

    sf::Color(100, 100, 255) 
}};

Surface::Segment::Segment()
    : vertices(sf::Quads, 4), isPassable(false)
{
}

bool Surface::Segment::operator==(const Segment &other) const
{
    return color == other.color && isPassable == other.isPassable;
}

Surface::Surface(Type type, const sf::Vector2f &position, const sf::Vector2f &size)
    : mType(type), mPosition(position), mSize(size)
{
    initializeSegments();
    updateGlobalBounds();

    switch (mType)
    {
    case Type::FLOOR:
        setMaterial(Material::SAND);
        break;
    case Type::CEILING:
        setMaterial(Material::STONE);
        break;
    case Type::WALL:
        setMaterial(Material::STONE);
        break;
    default:
        setMaterial(Material::STONE);
        break;
    }
}

void Surface::initializeSegments()
{
    const float segmentWidth = mSize.x / 3.0f;
    const float segmentHeight = mSize.y / 3.0f;

    sf::Color baseColor = MATERIAL_BASE_COLORS[static_cast<int>(mMaterial)];

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

            sf::Color segmentTint = SEGMENT_COLORS[y * 3 + x];
            segment.color = sf::Color(
                static_cast<sf::Uint8>(std::min<int>(255u, baseColor.r + segmentTint.r / 4)),
                static_cast<sf::Uint8>(std::min<int>(255u, baseColor.g + segmentTint.g / 4)),
                static_cast<sf::Uint8>(std::min<int>(255u, baseColor.b + segmentTint.b / 4)));

            for (int i = 0; i < 4; i++)
            {
                segment.vertices[i].color = segment.color;
            }

            if (mType == Type::WALL && x == 1 && y == 1)
            {
                segment.isPassable = false;
            }
            else
            {
                segment.isPassable = false;
            }
        }
    }
}

void Surface::draw(sf::RenderTarget &target) const
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
                    sf::RectangleShape debugRect(
                        sf::Vector2f(segment.hitbox.width, segment.hitbox.height));
                    debugRect.setPosition(segment.hitbox.left, segment.hitbox.top);
                    debugRect.setFillColor(sf::Color::Transparent);
                    debugRect.setOutlineColor(
                        segment.isPassable ? sf::Color::Green : sf::Color::Red);
                    debugRect.setOutlineThickness(0.03f);
                    target.draw(debugRect);
                }
            }
        }
    }
}

Surface::Segment *Surface::getSegmentAt(float localX, float localY)
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

const Surface::Segment *Surface::getSegmentAt(float localX, float localY) const
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

void Surface::setSegmentColor(int x, int y, const sf::Color &color)
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

void Surface::setAllSegmentsColor(const sf::Color &color)
{
    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            setSegmentColor(x, y, color);
        }
    }
}

sf::Color Surface::getSegmentColor(int x, int y) const
{
    if (x >= 0 && x < 3 && y >= 0 && y < 3)
    {
        return mSegments[y][x].color;
    }
    return sf::Color::Black;
}

void Surface::setSegmentPassable(int x, int y, bool passable)
{
    if (x >= 0 && x < 3 && y >= 0 && y < 3)
    {
        mSegments[y][x].isPassable = passable;
    }
}

bool Surface::isSegmentPassable(int x, int y) const
{
    if (x >= 0 && x < 3 && y >= 0 && y < 3)
    {
        return mSegments[y][x].isPassable;
    }
    return true;
}

void Surface::setSize(const sf::Vector2f &size)
{
    mSize = size;
    initializeSegments(); 
    updateGlobalBounds();
}

void Surface::updateGlobalBounds()
{
    mGlobalBounds = sf::FloatRect(mPosition.x, mPosition.y, mSize.x, mSize.y);
}

void Surface::setMaterial(Material material)
{
    mMaterial = material;

    sf::Color baseColor = MATERIAL_BASE_COLORS[static_cast<int>(mMaterial)];

    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            sf::Color segmentTint = SEGMENT_COLORS[y * 3 + x];
            sf::Color newColor = sf::Color(
                static_cast<sf::Uint8>(std::min<int>(255u, baseColor.r + segmentTint.r / 4)),
                static_cast<sf::Uint8>(std::min<int>(255u, baseColor.g + segmentTint.g / 4)),
                static_cast<sf::Uint8>(std::min<int>(255u, baseColor.b + segmentTint.b / 4)));
            setSegmentColor(x, y, newColor);
        }
    }
}

sf::Color Surface::getAverageColor() const
{
    sf::Color sum;
    int count = 0;
    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            auto color = mSegments[y][x].color;
            sum.r += color.r;
            sum.g += color.g;
            sum.b += color.b;
            count++;
        }
    }
    return sf::Color(
        static_cast<sf::Uint8>(sum.r / count),
        static_cast<sf::Uint8>(sum.g / count),
        static_cast<sf::Uint8>(sum.b / count));
}

sf::Color Surface::getBaseColor() const
{
    return MATERIAL_BASE_COLORS[static_cast<int>(mMaterial)];
}

Surface::LOD Surface::getLOD(float distance)
{
    if (distance < 5.0f)
        return LOD::HIGH;
    if (distance < 15.0f)
        return LOD::MEDIUM;
    return LOD::LOW;
}

sf::Color Surface::getColorAtSegment(int segX, int segY, float distance) const
{
    LOD lod = getLOD(distance);

    switch (lod)
    {
    case LOD::HIGH:
        return getSegmentColor(segX, segY);
    case LOD::MEDIUM:
        return getAverageColor();
    case LOD::LOW:
        return getBaseColor();
    }
    return sf::Color::White;
}

void Surface::updateVertices()
{
    const float segmentWidth = mSize.x / 3.0f;
    const float segmentHeight = mSize.y / 3.0f;

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

            for (int i = 0; i < 4; i++)
            {
                segment.vertices[i].color = segment.color;
            }

            if (mTextureAtlas)
            {
                float regionWidth = static_cast<float>(mTextureRegion.width);
                float regionHeight = static_cast<float>(mTextureRegion.height);

                for (int segY = 0; segY < 3; segY++)
                {
                    for (int segX = 0; segX < 3; segX++)
                    {
                        auto &segment = mSegments[segY][segX];

                        float u0 = mTextureRegion.left + (segX / 3.0f) * regionWidth;
                        float v0 = mTextureRegion.top + (segY / 3.0f) * regionHeight;
                        float u1 = mTextureRegion.left + ((segX + 1) / 3.0f) * regionWidth;
                        float v1 = mTextureRegion.top + ((segY + 1) / 3.0f) * regionHeight;

                        segment.vertices[0].texCoords = sf::Vector2f(u0, v0);
                        segment.vertices[1].texCoords = sf::Vector2f(u1, v0);
                        segment.vertices[2].texCoords = sf::Vector2f(u1, v1);
                        segment.vertices[3].texCoords = sf::Vector2f(u0, v1);
                    }
                }
            }
        }
    }

    updateGlobalBounds();
}

void Surface::setTextureAtlas(const sf::Texture *atlas)
{
    mTextureAtlas = atlas;
    updateVertices(); 
}

void Surface::setTextureRegion(const sf::IntRect &region)
{
    mTextureRegion = region;
    updateVertices(); 
}