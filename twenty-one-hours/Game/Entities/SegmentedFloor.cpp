#include "SegmentedFloor.h"

SegmentedFloor::SegmentedFloor(const sf::Vector2f& position, const sf::Vector2f& size)
    : mPosition(position), mSize(size), mColor(80, 80, 80) {
    mVertices.setPrimitiveType(sf::Quads);
    mVertices.resize(4);
    
    initializeVertices();
    updateGlobalBounds();
}

void SegmentedFloor::draw(sf::RenderTarget& target) const {
    target.draw(mVertices);
}

void SegmentedFloor::setColor(const sf::Color& color) {
    mColor = color;
    for (int i = 0; i < 4; i++) {
        mVertices[i].color = color;
    }
}

void SegmentedFloor::setTexture(const sf::Texture* texture) {
    mTexture = texture;
    if (mTexture) {
        mVertices[0].texCoords = sf::Vector2f(0, 0);
        mVertices[1].texCoords = sf::Vector2f(mTexture->getSize().x, 0);
        mVertices[2].texCoords = sf::Vector2f(mTexture->getSize().x, mTexture->getSize().y);
        mVertices[3].texCoords = sf::Vector2f(0, mTexture->getSize().y);
    }
}

void SegmentedFloor::initializeVertices() {
    mVertices[0].position = sf::Vector2f(mPosition.x, mPosition.y);
    mVertices[1].position = sf::Vector2f(mPosition.x + mSize.x, mPosition.y);
    mVertices[2].position = sf::Vector2f(mPosition.x + mSize.x, mPosition.y + mSize.y);
    mVertices[3].position = sf::Vector2f(mPosition.x, mPosition.y + mSize.y);
    
    for (int i = 0; i < 4; i++) {
        mVertices[i].color = mColor;
    }
}

void SegmentedFloor::updateGlobalBounds() {
    mGlobalBounds = sf::FloatRect(mPosition.x, mPosition.y, mSize.x, mSize.y);
}