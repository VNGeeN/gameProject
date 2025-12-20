#include "CollisionLayer.h"

CollisionLayer::CollisionLayer(int width, int height) 
    : mWidth(width), mHeight(height) {
    mCollisionGrid.resize(height, std::vector<bool>(width, false));
}

void CollisionLayer::setCollision(int x, int y, bool collidable) {
    if (x >= 0 && x < mWidth && y >= 0 && y < mHeight) {
        mCollisionGrid[y][x] = collidable;
    }
}

bool CollisionLayer::checkCollision(float x, float y) const {
    int cellX = static_cast<int>(x);
    int cellY = static_cast<int>(y);
    
    if (cellX >= 0 && cellX < mWidth && cellY >= 0 && cellY < mHeight) {
        return mCollisionGrid[cellY][cellX];
    }
    return true; 
}

bool CollisionLayer::checkCollision(const sf::FloatRect& rect) const {
    float corners[4][2] = {
        {rect.left, rect.top},
        {rect.left + rect.width, rect.top},
        {rect.left, rect.top + rect.height},
        {rect.left + rect.width, rect.top + rect.height}
    };
    
    for (int i = 0; i < 4; i++) {
        if (checkCollision(corners[i][0], corners[i][1])) {
            return true;
        }
    }
    return false;
}

void CollisionLayer::addDynamicHitbox(const sf::FloatRect& hitbox) {
    mDynamicHitboxes.push_back(hitbox);
}

void CollisionLayer::removeDynamicHitbox(const sf::FloatRect& hitbox) {
    mDynamicHitboxes.erase(
        std::remove(mDynamicHitboxes.begin(), mDynamicHitboxes.end(), hitbox),
        mDynamicHitboxes.end()
    );
}

bool CollisionLayer::checkDynamicCollision(const sf::FloatRect& rect) const {
    for (const auto& hitbox : mDynamicHitboxes) {
        if (hitbox.intersects(rect)) {
            return true;
        }
    }
    return false;
}