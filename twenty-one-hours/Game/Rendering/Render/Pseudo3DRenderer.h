#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "../Ray/RayCalc.h"
#include "../Map/Map.h"
#include "../Player/Player.h"
#include "../Core/ObjectManager.h"
#include "Skybox/SkyboxRenderer.h"
#include "../Entities/Surface.h"
#include "../Entities/TextureManager.h"

class Pseudo3DRenderer
{
public:
    Pseudo3DRenderer(sf::RenderWindow &win, RayCalc &ray, Map &m, Player &p,
                     const sf::Texture *enemyTex = nullptr);
    void render();
    void renderWalls();
    void renderWallSlice(int column, const RayCalc::Ray &ray);
    void renderFloorAndCeiling();
    void renderObjects();
    void setCeilingEnabled(bool enabled) { mCeilingEnabled = enabled; }

    void setDebugMode(bool debug) { mDebugMode = debug; }
    bool getDebugMode() const { return mDebugMode; }

    void renderDebugInfo();

    SkyboxRenderer &getSkyboxRenderer() { return mSkyboxRenderer; }

    void renderSprite(const sf::Vector2f &spritePos,
                      const sf::Vector2f &playerPos,
                      const sf::Texture *texture,
                      const sf::IntRect &region,
                      float rotation = 0.0f,
                      float visibility = 0.0f,
                      float worldHeight = 1.0f,
                      const sf::Color &tint = sf::Color::White);

    const sf::Texture *getEnemyTexture() const { return mEnemyTexture; }

private:
    sf::RenderWindow &window;
    RayCalc &rayCalc;
    Map &map;
    Player &player;

    static constexpr float DEFAULT_VIEW_DISTANCE = 20.0f;

    sf::Texture mFloorTexture;
    sf::Texture mCeilingTexture;
    sf::IntRect mFloorRegion;
    sf::IntRect mCeilingRegion;

    float calculateWallHeight(float distance) const;
    float calculateObjectScale(float distance);
    float calculateBrightness(float distance) const;
    float getViewDistance() const;

    sf::Vector2f calculateScreenPosition(sf::Vector2f worldPos, float distance);
    sf::Color getWallColor(const RayCalc::Ray &ray);
    sf::Color getFloorColor(float worldX, float worldY) const;
    sf::Color getCeilingColor(float worldX, float worldY) const;

    float getDistanceToPlayer(GameObject *obj);
    void renderObject(GameObject *obj, const ObjectVisibilityData &data);

    bool mDebugMode = false;
    sf::Font mDebugFont;

    void renderFloor();
    void renderCeiling();

    sf::Vector2f calculateWorldPosForFloorCeiling(int screenX, int screenY, bool isFloor) const;

    const sf::Texture *mEnemyTexture = nullptr;

    SkyboxRenderer mSkyboxRenderer;

    bool mCeilingEnabled = true;
};