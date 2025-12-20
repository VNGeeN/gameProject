#include "SegmentedCeiling.h"

SegmentedCeiling::SegmentedCeiling(const sf::Vector2f& position, const sf::Vector2f& size)
    : mPosition(position), mSize(size), mColor(60, 60, 100) {
    mVertices.setPrimitiveType(sf::Quads);
    mVertices.resize(4);
    
    initializeVertices();
    updateGlobalBounds();
}

void SegmentedCeiling::draw(sf::RenderTarget& target) const {
    target.draw(mVertices);
}

void SegmentedCeiling::setColor(const sf::Color& color) {
    mColor = color;
    for (int i = 0; i < 4; i++) {
        mVertices[i].color = color;
    }
}

void SegmentedCeiling::initializeVertices() {
    mVertices[0].position = sf::Vector2f(mPosition.x, mPosition.y);
    mVertices[1].position = sf::Vector2f(mPosition.x + mSize.x, mPosition.y);
    mVertices[2].position = sf::Vector2f(mPosition.x + mSize.x, mPosition.y + mSize.y);
    mVertices[3].position = sf::Vector2f(mPosition.x, mPosition.y + mSize.y);
    
    for (int i = 0; i < 4; i++) {
        mVertices[i].color = mColor;
    }
}

void SegmentedCeiling::updateGlobalBounds() {
    mGlobalBounds = sf::FloatRect(mPosition.x, mPosition.y, mSize.x, mSize.y);
}