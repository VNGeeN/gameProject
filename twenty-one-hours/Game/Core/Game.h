#pragma once
#include <SFML/Graphics.hpp>
#include "../Map/Map.h"
#include "../Player/Player.h"
#include "../Ray/RayCalc.h"
#include "../Rendering/Render/Pseudo3DRenderer.h"
#include "ObjectManager.h"
#include "TextureManager.h"
#include <memory>
#include <vector>
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
    void handleLevelTransitions(sf::Time deltaTime);
    void renderHud();
    void renderWeapon();
    void renderCrosshair();
    void renderHitMarker();
    void renderWorldInteractions2D();
    void renderMainMenu();
    void renderPauseMenu();
    void renderVictoryScreen();
    void renderDefeatScreen();
    void handleMenuInput(const sf::Event &event);
    void updateWeaponAnimation(sf::Time deltaTime, bool isMoving);
    void startNewGame();

    enum class GameState
    {
        MainMenu,
        Playing,
        Paused,
        Victory,
        Defeat
    };

    sf::RenderWindow mWindow;

    std::unique_ptr<Map> mMap;
    std::unique_ptr<Player> mPlayer;
    std::unique_ptr<RayCalc> mRayCalc;
    std::unique_ptr<Pseudo3DRenderer> mRenderer;

    sf::Font mUiFont;
    GameState mState = GameState::MainMenu;
    int mMainMenuIndex = 0;
    int mPauseMenuIndex = 0;
    int mLives = 3;

    bool mDebugMode = false;
    void toggleDebugMode();

    bool mDebug2DMode = false;
    void render2D();

    bool mChunkDebugMode = false;
    void toggleChunkDebugMode();

    void drawGrid();

    std::unique_ptr<EnemyManager> mEnemyManager;

    sf::Texture mEnemyTexture;
    sf::Texture mBossTexture;
    sf::Texture mWeaponTexture;
    bool mWeaponTextureLoaded = false;

    float mTransitionCooldown = 0.0f;

    float mWeaponKick = 0.0f;
    float mHitMarkerTimer = 0.0f;
    float mPickupMessageTimer = 0.0f;

    enum class WeaponAnimState
    {
        Idle,
        Walk,
        Shoot
    };

    WeaponAnimState mWeaponAnimState = WeaponAnimState::Idle;
    std::vector<sf::IntRect> mWeaponIdleFrames;
    std::vector<sf::IntRect> mWeaponWalkFrames;
    std::vector<sf::IntRect> mWeaponShootFrames;
    std::size_t mWeaponFrameIndex = 0;
    float mWeaponFrameTimer = 0.0f;
    float mWeaponShootTimer = 0.0f;
    float mWeaponBobPhase = 0.0f;

    float mShotShakeTimer = 0.0f;
    float mStepShakeTimer = 0.0f;
    float mStepIntervalTimer = 0.0f;
    float mShakePhase = 0.0f;
    sf::Vector2f mShakeOffset = sf::Vector2f(0.0f, 0.0f);
};