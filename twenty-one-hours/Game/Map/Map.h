#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Surface.h"
#include "CollisionLayer.h"
#include "ChunkManager.h"
#include "TextureManager.h"

class Map
{
public:
    enum class LevelType
    {
        Dungeon,
        OpenWorld
    };

    Map();

    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }
    char getTile(int x, int y) const;
    LevelType getLevelType() const { return mLevelType; }

    Surface *getWallAt(float x, float y) const;
    Surface *getFloorAt(float x, float y) const;
    Surface *getCeilingAt(float x, float y) const;

    Surface *getSurfaceAt(float x, float y, Surface::Type type) const;

    bool isWall(float x, float y) const;
    bool checkCollision(float x, float y, float radius = 0.3f) const;
    bool checkSegmentCollision(float worldX, float worldY) const;

    void drawWalls(sf::RenderTarget &target) const;
    void drawFloors(sf::RenderTarget &target) const;
    void drawCeilings(sf::RenderTarget &target) const;

    sf::Color getFloorColorAt(float x, float y) const;
    sf::Color getCeilingColorAt(float x, float y) const;

    GameObject *getObjectAt(float x, float y);

    void updateVisibleChunks(float x, float y);

    Surface *getSurfaceDirect(int x, int y, Surface::Type type) const
    {
        if (x >= 0 && x < mWidth && y >= 0 && y < mHeight)
        {
            return mCells[y][x].get(type);
        }
        return nullptr;
    }

    sf::Vector2f findPlayerStartPosition() const;

    bool tryGetTransitionTarget(float x, float y, LevelType &outTarget) const;

    struct Transition
    {
        sf::FloatRect triggerArea;
        sf::Vector2f destinationSpawn;
        LevelType target;
    };

    const std::vector<Transition> &getTransitions() const { return mTransitions; }

    bool tryGetTransitionTarget(float x, float y, LevelType &outTarget, sf::Vector2f &outSpawn) const;

    void regenerate(LevelType levelType);

    struct Door
    {
        sf::FloatRect area;
        LevelType target;
    };

    struct AmmoPickup
    {
        sf::Vector2f position;
        bool collected = false;
    };

    const std::vector<Door> &getDoors() const { return mDoors; }
    const std::vector<AmmoPickup> &getAmmoPickups() const { return mAmmoPickups; }
    bool tryCollectAmmoPickup(float x, float y);

private:
    struct Cell
    {
        std::unique_ptr<Surface> floor;
        std::unique_ptr<Surface> ceiling;
        std::unique_ptr<Surface> wall;

        Surface *get(Surface::Type type) const
        {
            switch (type)
            {
            case Surface::Type::FLOOR:
                return floor.get();
            case Surface::Type::CEILING:
                return ceiling.get();
            case Surface::Type::WALL:
                return wall.get();
            default:
                return nullptr;
            }
        }
    };

    void initializeBaseGrid();
    void initializeCells();
    void initializeCollisionLayer();
    void initializeSurfaces();
    void generateDungeonBaseGrid();
    void generateOpenWorldBaseGrid();
    void addTransition(int x, int y, LevelType target, const sf::Vector2f &destinationSpawn,
                       const sf::Vector2f &triggerSize = sf::Vector2f(0.9f, 0.9f));
    void carveFloorRect(int x, int y, int w, int h);
    void spawnAmmoPickups(int count);

    bool inBounds(int x, int y) const
    {
        return x >= 0 && x < mWidth && y >= 0 && y < mHeight;
    }

    int mWidth = 10;
    int mHeight = 10;
    LevelType mLevelType = LevelType::Dungeon;
    sf::Vector2f mPlayerStart{0.0f, 0.0f};
    std::vector<std::vector<char>> mBaseGrid;
    std::vector<std::vector<Cell>> mCells;
    CollisionLayer mCollisionLayer;

    std::unique_ptr<ChunkManager> mChunkManager;
    std::vector<Transition> mTransitions;
    std::vector<Door> mDoors;
    std::vector<AmmoPickup> mAmmoPickups;

    void createRoom(int x, int y, int w, int h);
    void createHorizontalCorridor(int x1, int x2, int y, int width = 2);
    void createVerticalCorridor(int x, int y1, int y2, int width = 2);
};