#include "Map.h"
#include <iostream>

Map::Map() : mCollisionLayer(10, 10)
{
    initializeBaseGrid();
    initializeCollisionLayer();
    initializeSurfaces();
    // mChunkManager = std::make_unique<ChunkManager>(*this);
}

void Map::initializeBaseGrid()
{
    mBaseGrid = {
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '#', '#', '.', '#', '#', '.', '.', '#'},
        {'#', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
        {'#', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
        {'#', '.', '.', '.', '#', '.', '.', '.', '.', '#'},
        {'#', '.', '#', '.', '#', '.', '#', '.', '.', '#'},
        {'#', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}};
    mWidth = mBaseGrid[0].size();
    mHeight = mBaseGrid.size();
}

void Map::initializeCollisionLayer()
{
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            mCollisionLayer.setCollision(x, y, (mBaseGrid[y][x] == '#'));
        }
    }
}

void Map::initializeCells()
{
    mCells.resize(mHeight);

    for (int y = 0; y < mHeight; y++)
    {
        mCells[y].resize(mWidth);

        for (int x = 0; x < mWidth; x++)
        {
            sf::Vector2f position(static_cast<float>(x), static_cast<float>(y));
            auto &cell = mCells[y][x];

            cell.floor = std::make_unique<Surface>(Surface::Type::FLOOR, position);
            cell.floor->setMaterial(Surface::Material::SAND);

            cell.ceiling = std::make_unique<Surface>(Surface::Type::CEILING, position);
            cell.ceiling->setMaterial(Surface::Material::STONE);

            if (mBaseGrid[y][x] == '#')
            {
                cell.wall = std::make_unique<Surface>(Surface::Type::WALL, position);
                cell.wall->setMaterial(Surface::Material::STONE);
            }
        }
    }
}

char Map::getTile(int x, int y) const
{
    if (inBounds(x, y))
    {
        return mBaseGrid[y][x];
    }
    return '#';
}

Surface *Map::getWallAt(float x, float y) const
{
    int cellX = static_cast<int>(x);
    int cellY = static_cast<int>(y);
    if (inBounds(cellX, cellY))
    {
        return mCells[cellY][cellX].wall.get();
    }
    return nullptr;
}

Surface *Map::getFloorAt(float x, float y) const
{
    int cellX = static_cast<int>(x);
    int cellY = static_cast<int>(y);
    if (inBounds(cellX, cellY))
    {
        return mCells[cellY][cellX].floor.get();
    }
    return nullptr;
}

Surface *Map::getCeilingAt(float x, float y) const
{
    int cellX = static_cast<int>(x);
    int cellY = static_cast<int>(y);
    if (inBounds(cellX, cellY))
    {
        return mCells[cellY][cellX].ceiling.get();
    }
    return nullptr;
}

Surface *Map::getSurfaceAt(float x, float y, Surface::Type type) const
{
    int cellX = static_cast<int>(x);
    int cellY = static_cast<int>(y);
    if (inBounds(cellX, cellY))
    {
        return mCells[cellY][cellX].get(type);
    }
    return nullptr;
}

bool Map::isWall(float x, float y) const
{
    return mCollisionLayer.checkCollision(x, y);
}

bool Map::checkCollision(float x, float y, float radius) const
{
    sf::FloatRect rect(x - radius, y - radius, radius * 2, radius * 2);
    return mCollisionLayer.checkCollision(rect);
}

bool Map::checkSegmentCollision(float worldX, float worldY) const
{
    int cellX = static_cast<int>(worldX);
    int cellY = static_cast<int>(worldY);

    if (!inBounds(cellX, cellY))
        return true; 

    auto wall = mCells[cellY][cellX].wall.get();
    if (!wall)
        return false; 

    float localX = worldX - cellX;
    float localY = worldY - cellY;

    auto segment = wall->getSegmentAt(localX, localY);
    return segment && !segment->isPassable; 
}

void Map::drawWalls(sf::RenderTarget &target) const
{
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            auto wall = mCells[y][x].wall.get();
            if (wall)
            {
                wall->draw(target);
            }
        }
    }
}

void Map::drawFloors(sf::RenderTarget &target) const
{
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            auto floor = mCells[y][x].floor.get();
            if (floor)
            {
                floor->draw(target);
            }
        }
    }
}

void Map::drawCeilings(sf::RenderTarget &target) const
{
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            auto ceiling = mCells[y][x].ceiling.get();
            if (ceiling)
            {
                ceiling->draw(target);
            }
        }
    }
}

sf::Color Map::getFloorColorAt(float x, float y) const
{
    auto floor = getFloorAt(x, y);
    if (floor)
    {
        return floor->getAverageColor();
    }
    return sf::Color(210, 180, 140); 
}

sf::Color Map::getCeilingColorAt(float x, float y) const
{
    auto ceiling = getCeilingAt(x, y);
    if (ceiling)
    {
        return ceiling->getAverageColor();
    }
    return sf::Color(100, 100, 100); 
}

GameObject *Map::getObjectAt(float x, float y)
{
    return getWallAt(x, y);
}

void Map::updateVisibleChunks(float x, float y)
{
    if (!mChunkManager)
    {
        mChunkManager = std::make_unique<ChunkManager>(*this);
    }
    mChunkManager->update(sf::Vector2f(x, y));
}

void Map::initializeSurfaces()
{
    int height = mBaseGrid.size();
    int width = mBaseGrid[0].size();

    std::cout << "[Map] initializeSurfaces() start" << std::endl;
    auto &tm = TextureManager::getInstance();
    const sf::Texture *atlas = tm.getAtlas("main");
    std::cout << "[Map] Atlas ptr = " << atlas << std::endl;

    mCells.resize(height);

    for (int y = 0; y < height; y++)
    {
        mCells[y].resize(width);

        for (int x = 0; x < width; x++)
        {
            sf::Vector2f position(static_cast<float>(x), static_cast<float>(y));
            auto &cell = mCells[y][x];

            cell.floor = std::make_unique<Surface>(Surface::Type::FLOOR, position);
            cell.floor->setMaterial(Surface::Material::SAND);
            if (atlas)
            {
                cell.floor->setTextureAtlas(atlas);
                cell.floor->setTextureRegion(tm.getRegion("SAND_FLOOR"));
            }

            cell.ceiling = std::make_unique<Surface>(Surface::Type::CEILING, position);
            cell.ceiling->setMaterial(Surface::Material::STONE);
            if (atlas)
            {
                cell.ceiling->setTextureAtlas(atlas);
                cell.ceiling->setTextureRegion(tm.getRegion("STONE_CEIL"));
            }

            if (mBaseGrid[y][x] == '#')
            {
                cell.wall = std::make_unique<Surface>(Surface::Type::WALL, position);
                cell.wall->setMaterial(Surface::Material::STONE);
                if (atlas)
                {
                    cell.wall->setTextureAtlas(atlas);
                    cell.wall->setTextureRegion(tm.getRegion("STONE_WALL"));
                }
            }
        }
    }
}