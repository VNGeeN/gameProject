#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <memory>
#include "GameObject.h"

class SegmentedWall : public GameObject {
public:
    struct Segment {
        sf::VertexArray vertices;
        sf::FloatRect hitbox;
        sf::Color color;
        bool isPassable;
        
        Segment() : vertices(sf::Quads, 4), isPassable(false) {}
    };

    SegmentedWall(const sf::Vector2f& position, const sf::Vector2f& size = sf::Vector2f(1.0f, 1.0f));
    
    sf::Vector2f getPosition() const override { return mPosition; }
    void update(float deltaTime) override {}  
    void draw(sf::RenderTarget& target) const override;  
    sf::FloatRect getBoundingBox() const override { return mGlobalBounds; }
    bool isStatic() const override { return true; }
    
    Segment* getSegmentAt(float localX, float localY);
    const Segment* getSegmentAt(float localX, float localY) const;
    bool checkRayCollision(const sf::Vector2f& rayStart, const sf::Vector2f& rayDir, 
                          float& distance, int& segmentX, int& segmentY) const;
    
    void setSegmentColor(int x, int y, const sf::Color& color);
    void setAllSegmentsColor(const sf::Color& color);
    sf::Color getSegmentColor(int x, int y) const;
    
    void setSegmentPassable(int x, int y, bool passable);
    bool isSegmentPassable(int x, int y) const;
    
    sf::Vector2f getSize() const { return mSize; }
    void setSize(const sf::Vector2f& size);
    
    int getTotalSegments() const { return 9; }
    void setDebugMode(bool debug) { mDebugMode = debug; }

private:
    void initializeSegments();
    void updateSegmentVertices(int x, int y);
    void updateGlobalBounds();
    bool checkSegmentRayCollision(const Segment& segment, const sf::Vector2f& rayStart, 
                                 const sf::Vector2f& rayDir, float& distance) const;
    
    std::array<std::array<Segment, 3>, 3> mSegments;
    sf::Vector2f mPosition;
    sf::Vector2f mSize;
    sf::FloatRect mGlobalBounds;
    bool mDebugMode = true;
    
    static constexpr float SEGMENT_SIZE = 1.0f / 3.0f;
};