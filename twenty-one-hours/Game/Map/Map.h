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
    Map();

    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }
    char getTile(int x, int y) const;

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

    bool inBounds(int x, int y) const
    {
        return x >= 0 && x < mWidth && y >= 0 && y < mHeight;
    }

    int mWidth = 10;
    int mHeight = 10;
    std::vector<std::vector<char>> mBaseGrid;
    std::vector<std::vector<Cell>> mCells;
    CollisionLayer mCollisionLayer;

    std::unique_ptr<ChunkManager> mChunkManager;
};