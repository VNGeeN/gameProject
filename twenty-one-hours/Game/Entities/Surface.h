#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <memory>
#include "GameObject.h"

class Surface : public GameObject
{
public:
    enum class Type
    {
        WALL,    
        FLOOR,   
        CEILING, 
        SLOPE    
    };

    enum class Material
    {
        SAND,
        STONE,
        METAL,
        WOOD,
        GRASS
    };

    struct Segment
    {
        sf::VertexArray vertices;
        sf::FloatRect hitbox;
        sf::Color color;
        bool isPassable;

        Segment();
        bool operator==(const Segment &other) const;
    };

    Surface(Type type, const sf::Vector2f &position,
            const sf::Vector2f &size = sf::Vector2f(1.0f, 1.0f));

    sf::Vector2f getPosition() const override { return mPosition; }
    void update(float deltaTime) override {}
    void draw(sf::RenderTarget &target) const override;
    sf::FloatRect getBoundingBox() const override { return mGlobalBounds; }
    bool isStatic() const override { return true; }

    Segment *getSegmentAt(float localX, float localY);
    const Segment *getSegmentAt(float localX, float localY) const;

    void setSegmentColor(int x, int y, const sf::Color &color);
    void setAllSegmentsColor(const sf::Color &color);
    sf::Color getSegmentColor(int x, int y) const;

    void setSegmentPassable(int x, int y, bool passable);
    bool isSegmentPassable(int x, int y) const;

    void setMaterial(Material material);
    Material getMaterial() const { return mMaterial; }

    sf::Color getColorAtSegment(int segX, int segY, float distance) const;

    Type getType() const { return mType; }

    sf::Vector2f getSize() const { return mSize; }
    void setSize(const sf::Vector2f &size);

    void setDebugMode(bool debug) { mDebugMode = debug; }
    bool getDebugMode() const { return mDebugMode; }

    enum class LOD
    {
        HIGH,   
        MEDIUM, 
        LOW     
    };

    static LOD getLOD(float distance);

    sf::Color getAverageColor() const;
    sf::Color getBaseColor() const;

    void setTextureAtlas(const sf::Texture* atlas);
    const sf::Texture* getTextureAtlas() const { return mTextureAtlas; }
    
    void setTextureRegion(const sf::IntRect& region);
    const sf::IntRect& getTextureRegion() const { return mTextureRegion; }

private:
    void initializeSegments();
    void updateSegmentGeometry(int x, int y);
    void updateGlobalBounds();
    void updateVertices();

    Type mType;

    sf::Vector2f mPosition;
    sf::Vector2f mSize;
    sf::FloatRect mGlobalBounds;

    std::array<std::array<Segment, 3>, 3> mSegments;

    Material mMaterial = Material::STONE;

    bool mDebugMode = false;

    static constexpr float SEGMENT_SIZE = 1.0f / 3.0f;
    static const std::array<sf::Color, 5> MATERIAL_BASE_COLORS;
    static const std::array<sf::Color, 9> SEGMENT_COLORS;

    const sf::Texture *mTextureAtlas = nullptr;
    sf::IntRect mTextureRegion = sf::IntRect(0, 0, 512, 512);
};