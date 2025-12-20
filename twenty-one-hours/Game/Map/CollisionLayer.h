#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

class CollisionLayer {
public:
    CollisionLayer(int width, int height);
    
    void setCollision(int x, int y, bool collidable);
    bool checkCollision(float x, float y) const;
    bool checkCollision(const sf::FloatRect& rect) const;
    
    void addDynamicHitbox(const sf::FloatRect& hitbox);
    void removeDynamicHitbox(const sf::FloatRect& hitbox);
    bool checkDynamicCollision(const sf::FloatRect& rect) const;
    
    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }

private:
    std::vector<std::vector<bool>> mCollisionGrid;
    std::vector<sf::FloatRect> mDynamicHitboxes;
    int mWidth, mHeight;
};