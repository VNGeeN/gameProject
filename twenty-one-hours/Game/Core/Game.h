#pragma once
#include <SFML/Graphics.hpp>
#include "../Map/Map.h"
#include "../Player/Player.h"
#include "../Ray/RayCalc.h"
#include "../Rendering/Render/Pseudo3DRenderer.h"
#include "ObjectManager.h"
#include "TextureManager.h"
#include <memory>

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(sf::Time deltaTime);
    void render();
    void handleInput(sf::Time deltaTime);

    sf::RenderWindow mWindow;
    // Map mMap;
    // Player mPlayer;
    // RayCalc mRayCalc;
    // Pseudo3DRenderer mRenderer;
    std::unique_ptr<Map> mMap;
    std::unique_ptr<Player> mPlayer;
    std::unique_ptr<RayCalc> mRayCalc;
    std::unique_ptr<Pseudo3DRenderer> mRenderer;
    //ObjectManager mObjectManager;

    bool mDebugMode = false;
    void toggleDebugMode();

    bool mDebug2DMode = false; 
    void render2D();           
};