#include "Chunk.h"
#include "Map.h"
#include <cmath>

Chunk::Chunk(const Coord& coord, Map& map) 
    : mCoord(coord), mMap(map) {
}

Surface* Chunk::getSurfaceAtLocal(int localX, int localY, Surface::Type type) const {
    if (localX < 0 || localX >= CHUNK_SIZE ||
        localY < 0 || localY >= CHUNK_SIZE)
    {
        return nullptr;
    }

    int globalX = mCoord.x * CHUNK_SIZE + localX;
    int globalY = mCoord.y * CHUNK_SIZE + localY;
    
    return mMap.getSurfaceDirect(globalX, globalY, type);
}


sf::FloatRect Chunk::getBounds() const {
    float left = static_cast<float>(mCoord.x * CHUNK_SIZE);
    float top = static_cast<float>(mCoord.y * CHUNK_SIZE);
    return sf::FloatRect(left, top, 
                        static_cast<float>(CHUNK_SIZE), 
                        static_cast<float>(CHUNK_SIZE));
}

void Chunk::drawWalls(sf::RenderTarget &target) const
{
    for (int y = 0; y < CHUNK_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            int globalX = mCoord.x * CHUNK_SIZE + x;
            int globalY = mCoord.y * CHUNK_SIZE + y;
            
            auto wall = mMap.getSurfaceDirect(globalX, globalY, Surface::Type::WALL);
            if (wall)
            {
                wall->draw(target);
            }
        }
    }
}

void Chunk::drawFloors(sf::RenderTarget &target) const
{
    for (int y = 0; y < CHUNK_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            int globalX = mCoord.x * CHUNK_SIZE + x;
            int globalY = mCoord.y * CHUNK_SIZE + y;
            
            auto floor = mMap.getSurfaceDirect(globalX, globalY, Surface::Type::FLOOR);
            if (floor)
            {
                floor->draw(target);
            }
        }
    }
}

void Chunk::drawCeilings(sf::RenderTarget &target) const
{
    for (int y = 0; y < CHUNK_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            int globalX = mCoord.x * CHUNK_SIZE + x;
            int globalY = mCoord.y * CHUNK_SIZE + y;
            
            auto ceiling = mMap.getSurfaceDirect(globalX, globalY, Surface::Type::CEILING);
            if (ceiling)
            {
                ceiling->draw(target);
            }
        }
    }
}
