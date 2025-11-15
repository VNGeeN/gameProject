#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include "../Ray/RayCalc.h"
#include "../Map/Map.h"
#include "../Player/Player.h"
#include "../GameObject/GameObject.h"
#include "../Data/ObjectVisibilityData.h"
#include "../Textures/TextureManager.h"

class Pseudo3DRenderer
{

public:
    Pseudo3DRenderer(sf::RenderWindow &win, RayCalc &ray, Map &m, Player &p);
    void render();
    void renderWalls();
    //void renderWallSlice(int column, const RayCalc::Ray &ray);
    void renderWallSlice(int column, const RayCalc::Ray &ray, sf::VertexArray& vertices, int vertexIndex);
    void renderFloorAndCeiling();
    void renderObjects();

private:
    sf::RenderWindow &window;
    RayCalc &rayCalc;
    Map &map;
    Player &player;

    // Текстуры
    sf::Texture wallTexture;
    sf::Texture floorTexture;
    sf::Texture ceilingTexture;

    static constexpr float MAX_VIEW_DISTANCE = 20.0f;

    float calculateWallHeight(float distance);
    float calculateObjectScale(float distance);
    float calculateBrightness(float distance);
    sf::Vector2f calculateScreenPosition(sf::Vector2f worldPos, float distance);
    sf::Color getWallColor(const RayCalc::Ray &ray);
    float getDistanceToPlayer(GameObject *obj);
    void renderObject(GameObject *obj, const ObjectVisibilityData &data);

    // Методы загрузки текстур
    void loadTextures();
    void createDefaultWallTexture();
    void createDefaultFloorTexture();
    void createDefaultCeilingTexture();
};
