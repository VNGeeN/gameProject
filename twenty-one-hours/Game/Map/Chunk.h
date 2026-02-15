#pragma once
#include <SFML/Graphics.hpp>
#include "Surface.h"

class Map;

class Chunk
{
public:
    static constexpr int CHUNK_SIZE = 8; 

    struct Coord
    {
        int x, y;

        bool operator==(const Coord &other) const
        {
            return x == other.x && y == other.y;
        }

        bool operator!=(const Coord &other) const
        {
            return !(*this == other);
        }
    };

    Chunk(const Coord &coord, Map &map);

    Surface *getSurfaceAtLocal(int localX, int localY, Surface::Type type) const;

    void drawWalls(sf::RenderTarget &target) const;
    void drawFloors(sf::RenderTarget &target) const;
    void drawCeilings(sf::RenderTarget &target) const;

    const Coord &getCoord() const { return mCoord; }
    sf::FloatRect getBounds() const;
    bool isLoaded() const { return true; }

    int toGlobalX(int localX) const { return mCoord.x * CHUNK_SIZE + localX; }
    int toGlobalY(int localY) const { return mCoord.y * CHUNK_SIZE + localY; }

private:
    Coord mCoord;
    Map &mMap; 
};