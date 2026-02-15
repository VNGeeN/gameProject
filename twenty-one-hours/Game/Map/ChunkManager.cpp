#include "ChunkManager.h"
#include "Map.h"
#include <cmath>
#include <algorithm>

ChunkManager::ChunkManager(Map &map) : mMap(map)
{
}

void ChunkManager::update(const sf::Vector2f &playerPos)
{

    float dx = playerPos.x - mLastPlayerPos.x;
    float dy = playerPos.y - mLastPlayerPos.y;
    if (dx * dx + dy * dy < 4.0f)
    { 
        return;
    }

    mLastPlayerPos = playerPos;
    calculateVisibleChunks(playerPos);
}

Chunk *ChunkManager::getChunk(const Chunk::Coord &coord)
{
    size_t hash = CoordHash{}(coord);
    auto it = mChunkCache.find(hash);

    if (it == mChunkCache.end())
    {
        auto chunk = std::make_unique<Chunk>(coord, mMap);
        it = mChunkCache.emplace(hash, std::move(chunk)).first;
    }

    return it->second.get();
}

Chunk *ChunkManager::getChunkAtWorld(float worldX, float worldY)
{
    Chunk::Coord coord;
    coord.x = static_cast<int>(std::floor(worldX / Chunk::CHUNK_SIZE));
    coord.y = static_cast<int>(std::floor(worldY / Chunk::CHUNK_SIZE));
    return getChunk(coord);
}

Surface *ChunkManager::getSurfaceAt(float worldX, float worldY, Surface::Type type)
{
    Chunk *chunk = getChunkAtWorld(worldX, worldY);
    if (!chunk)
        return nullptr;

    int globalX = static_cast<int>(worldX);
    int globalY = static_cast<int>(worldY);

    int chunkX = static_cast<int>(std::floor(worldX / Chunk::CHUNK_SIZE));
    int chunkY = static_cast<int>(std::floor(worldY / Chunk::CHUNK_SIZE));

    int localX = ((globalX % Chunk::CHUNK_SIZE) + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;
    int localY = ((globalY % Chunk::CHUNK_SIZE) + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;

    return chunk->getSurfaceAtLocal(localX, localY, type);
}

void ChunkManager::calculateVisibleChunks(const sf::Vector2f &playerPos)
{
    mVisibleChunks.clear();

    int playerChunkX = static_cast<int>(std::floor(playerPos.x / Chunk::CHUNK_SIZE));
    int playerChunkY = static_cast<int>(std::floor(playerPos.y / Chunk::CHUNK_SIZE));

    int radius = mViewDistance;

    for (int dy = -radius; dy <= radius; dy++)
    {
        for (int dx = -radius; dx <= radius; dx++)
        {
            Chunk::Coord coord{playerChunkX + dx, playerChunkY + dy};

            if (isChunkVisible(coord, playerPos))
            {
                Chunk *chunk = getChunk(coord);
                mVisibleChunks.push_back(chunk);
            }
        }
    }
}

bool ChunkManager::isChunkVisible(const Chunk::Coord &coord,
                                  const sf::Vector2f &playerPos) const
{
    float chunkCenterX = (coord.x + 0.5f) * Chunk::CHUNK_SIZE;
    float chunkCenterY = (coord.y + 0.5f) * Chunk::CHUNK_SIZE;

    float dx = chunkCenterX - playerPos.x;
    float dy = chunkCenterY - playerPos.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    float maxDistance = (mViewDistance + 0.5f) * Chunk::CHUNK_SIZE;
    return distance <= maxDistance;
}