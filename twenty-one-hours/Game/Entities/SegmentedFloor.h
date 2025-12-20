#pragma once
#include <SFML/Graphics.hpp>
#include "GameObject.h"

class SegmentedFloor : public GameObject {
public:
    SegmentedFloor(const sf::Vector2f& position, const sf::Vector2f& size = sf::Vector2f(1.0f, 1.0f));
    
    sf::Vector2f getPosition() const override { return mPosition; }
    void update(float deltaTime) override {}
    void draw(sf::RenderTarget& target) const override;
    sf::FloatRect getBoundingBox() const override { return mGlobalBounds; }
    bool isStatic() const override { return true; }
    
    void setColor(const sf::Color& color);
    sf::Color getColor() const { return mColor; }
    
    void setTexture(const sf::Texture* texture);
    
private:
    void initializeVertices();
    void updateGlobalBounds();
    
    sf::VertexArray mVertices;
    sf::Vector2f mPosition;
    sf::Vector2f mSize;
    sf::FloatRect mGlobalBounds;
    sf::Color mColor;
    const sf::Texture* mTexture = nullptr;
};