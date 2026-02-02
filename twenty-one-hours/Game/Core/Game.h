#pragma once
#include <SFML/Graphics.hpp>
#include "../Map/Map.h"
#include "../Player/Player.h"
#include "../Ray/RayCalc.h"
#include "../Rendering/Render/Pseudo3DRenderer.h"
#include "ObjectManager.h"
#include "TextureManager.h"
#include <memory>
#include "EnemyManager.h"

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
    
    std::unique_ptr<Map> mMap;
    std::unique_ptr<Player> mPlayer;
    std::unique_ptr<RayCalc> mRayCalc;
    std::unique_ptr<Pseudo3DRenderer> mRenderer;
    

    bool mDebugMode = false;
    void toggleDebugMode();

    bool mDebug2DMode = false; 
    void render2D();  
    
    bool mChunkDebugMode = false;
    void toggleChunkDebugMode();

    void drawGrid();

    std::unique_ptr<EnemyManager> mEnemyManager;

    sf::Texture mEnemyTexture;
};