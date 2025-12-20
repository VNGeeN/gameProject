#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>

enum class SkyboxMode {
    Stretch,  
    Repeat    
};

class SkyboxRenderer {
public:
    SkyboxRenderer(sf::RenderWindow& window);
    
    bool loadFromFile(const std::string& filename);
    
    void render(float playerAngle, float horizonY);
    
    void setMode(SkyboxMode mode) { mMode = mode; }
    SkyboxMode getMode() const { return mMode; }
    
    void setParallaxSpeed(float speed) { mParallaxSpeed = speed; }
    float getParallaxSpeed() const { return mParallaxSpeed; }
    
    bool isLoaded() const { return mIsLoaded; }
    sf::Vector2u getTextureSize() const { return mTexture.getSize(); }

private:
    SkyboxMode mMode = SkyboxMode::Stretch;
    sf::RenderWindow& mWindow;
    sf::Texture mTexture;
    sf::Sprite mSprite;
    bool mIsLoaded = false;
    float mParallaxSpeed = 1.0f; 
};