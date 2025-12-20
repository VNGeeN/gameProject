#pragma once
#include <SFML/Graphics.hpp>

class GameObject {
public:
    virtual ~GameObject() = default;
    
    virtual sf::Vector2f getPosition() const = 0;
    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderTarget& target) const = 0;
    virtual sf::FloatRect getBoundingBox() const = 0;
    virtual bool isStatic() const { return true; }
    
    void setSpatialKey(uint32_t key) { mSpatialKey = key; }
    uint32_t getSpatialKey() const { return mSpatialKey; }
    
    void setDebugId(int id) { mDebugId = id; }
    int getDebugId() const { return mDebugId; }

private:
    uint32_t mSpatialKey = 0;
    int mDebugId = -1;
};