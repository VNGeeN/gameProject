#pragma once

#include <SFML/Graphics.hpp>
#include "Map/Map.h"
#include "Player/Player.h"
#include "Ray/RayCalc.h"
#include "Render/Pseudo3DRenderer.h"

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(sf::Time daltaTime);
    void render();

    sf::RenderWindow mWindow;
    Map mMap;
    Player mPlayer;
    RayCalc mRayCalc;
    Pseudo3DRenderer mRenderer;
};