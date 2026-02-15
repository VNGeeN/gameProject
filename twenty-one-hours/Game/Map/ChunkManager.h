#pragma once
#include <vector>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "Chunk.h"

class Map;

class ChunkManager {
public:
    ChunkManager(Map& map);
    
    void update(const sf::Vector2f& playerPos);
    
    Chunk* getChunk(const Chunk::Coord& coord);
    Chunk* getChunkAtWorld(float worldX, float worldY);
    
    Surface* getSurfaceAt(float worldX, float worldY, Surface::Type type);
    
    void setViewDistance(int chunks) { mViewDistance = chunks; }
    int getViewDistance() const { return mViewDistance; }

private:
    struct CoordHash {
        size_t operator()(const Chunk::Coord& c) const {
            return static_cast<size_t>(c.x) * 31 + static_cast<size_t>(c.y);
        }
    };

    Map& mMap;
    std::unordered_map<size_t, std::unique_ptr<Chunk>> mChunkCache;
    std::vector<Chunk*> mVisibleChunks;
    int mViewDistance = 2; 
    
    void calculateVisibleChunks(const sf::Vector2f& playerPos);
    bool isChunkVisible(const Chunk::Coord& coord, const sf::Vector2f& playerPos) const;

    sf::Vector2f mLastPlayerPos = {0, 0};
};