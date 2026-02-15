#include "Map.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <climits>

Map::Map() : mCollisionLayer(10, 10)
{
    regenerate(LevelType::Dungeon);
    // mChunkManager = std::make_unique<ChunkManager>(*this);
}

void Map::initializeBaseGrid()
{
    mTransitions.clear();
    mDoors.clear();
    mAmmoPickups.clear();

    if (mLevelType == LevelType::OpenWorld)
    {
        generateOpenWorldBaseGrid();
    }
    else
    {
        generateDungeonBaseGrid();
    }
}

void Map::createRoom(int x, int y, int w, int h)
{
    for (int dy = 0; dy < h; dy++)
    {
        for (int dx = 0; dx < w; dx++)
        {
            int px = x + dx;
            int py = y + dy;

            if (px >= 0 && px < mWidth && py >= 0 && py < mHeight)
            {
                mBaseGrid[py][px] = '.';
            }
        }
    }

    if (w > 6 && h > 6)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 100);

        int centerX = x + w / 2;
        int centerY = y + h / 2;

        for (int dy = 1; dy < h - 1; dy++)
        {
            for (int dx = 1; dx < w - 1; dx++)
            {
                int px = x + dx;
                int py = y + dy;

                if (std::abs(px - centerX) <= 1 && std::abs(py - centerY) <= 1)
                {
                    continue;
                }

                if (distrib(gen) < 15)
                {
                    mBaseGrid[py][px] = '#';
                }
            }
        }
    }
}

void Map::createHorizontalCorridor(int x1, int x2, int y, int width)
{
    std::cout << "[Map] Creating horizontal corridor from x=" << x1 << " to x=" << x2 << " at y=" << y << std::endl;

    int startX = std::min(x1, x2);
    int endX = std::max(x1, x2);

    for (int x = startX; x <= endX; x++)
    {
        for (int offset = 0; offset < width; offset++)
        {
            int corridorY = y + offset;
            if (x >= 0 && x < static_cast<int>(mBaseGrid[0].size()) &&
                corridorY >= 0 && corridorY < static_cast<int>(mBaseGrid.size()))
            {
                mBaseGrid[corridorY][x] = '.';
            }
        }
    }
}

void Map::createVerticalCorridor(int x, int y1, int y2, int width)
{
    std::cout << "[Map] Creating vertical corridor at x=" << x << " from y=" << y1 << " to y=" << y2 << std::endl;

    int startY = std::min(y1, y2);
    int endY = std::max(y1, y2);

    for (int y = startY; y <= endY; y++)
    {
        for (int offset = 0; offset < width; offset++)
        {
            int corridorX = x + offset;
            if (corridorX >= 0 && corridorX < static_cast<int>(mBaseGrid[0].size()) &&
                y >= 0 && y < static_cast<int>(mBaseGrid.size()))
            {
                mBaseGrid[y][corridorX] = '.';
            }
        }
    }
}

void Map::initializeCollisionLayer()
{
    mCollisionLayer = CollisionLayer(mWidth, mHeight);

    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            bool isWall = (mBaseGrid[y][x] == '#');
            mCollisionLayer.setCollision(x, y, isWall);
        }
    }

    std::cout << "[Map] CollisionLayer initialized: " << mWidth << "x" << mHeight << std::endl;
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
    if (mWidth <= 16 && mHeight <= 16)
    {
        int cellX = static_cast<int>(x);
        int cellY = static_cast<int>(y);
        if (inBounds(cellX, cellY))
        {
            return mCells[cellY][cellX].get(type);
        }
        return nullptr;
    }

    if (!mChunkManager)
    {
        const_cast<Map *>(this)->mChunkManager = std::make_unique<ChunkManager>(*const_cast<Map *>(this));
    }

    return mChunkManager->getSurfaceAt(x, y, type);
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
    int height = mHeight;
    int width = mWidth;

    std::cout << "[Map] initializeSurfaces() - размеры: "
              << width << "x" << height << std::endl;

    auto &tm = TextureManager::getInstance();
    const sf::Texture *atlas = tm.getAtlas("main");

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

            cell.ceiling = std::make_unique<Surface>(Surface::Type::CEILING, position);
            cell.ceiling->setMaterial(Surface::Material::STONE);

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
            else
            {
                cell.wall.reset();
            }

            if (atlas)
            {
                if (cell.floor)
                {
                    cell.floor->setTextureAtlas(atlas);
                    cell.floor->setTextureRegion(tm.getRegion("SAND_FLOOR"));
                }
                if (cell.ceiling)
                {
                    cell.ceiling->setTextureAtlas(atlas);
                    cell.ceiling->setTextureRegion(tm.getRegion("STONE_CEIL"));
                }
            }
        }
    }

    int wallCount = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (mBaseGrid[y][x] == '#')
                wallCount++;
        }
    }
    std::cout << "[Map] Стены: " << wallCount << ", проходы: "
              << (width * height - wallCount) << std::endl;
}

sf::Vector2f Map::findPlayerStartPosition() const
{
    return mPlayerStart;
}

bool Map::tryGetTransitionTarget(float x, float y, LevelType &outTarget) const
{
    sf::Vector2f ignoredSpawn;
    return tryGetTransitionTarget(x, y, outTarget, ignoredSpawn);
}

bool Map::tryGetTransitionTarget(float x, float y, LevelType &outTarget, sf::Vector2f &outSpawn) const
{
    for (const auto &transition : mTransitions)
    {
        if (transition.triggerArea.contains(x, y))
        {
            outTarget = transition.target;
            outSpawn = transition.destinationSpawn;
            return true;
        }
    }
    return false;
}

void Map::regenerate(LevelType levelType)
{
    mLevelType = levelType;
    mChunkManager.reset();
    initializeBaseGrid();
    initializeCollisionLayer();
    initializeSurfaces();

    if (mLevelType == LevelType::OpenWorld)
    {
        spawnAmmoPickups(26);
    }
    else
    {
        spawnAmmoPickups(14);
    }
}

void Map::generateDungeonBaseGrid()
{
    mWidth = 60;
    mHeight = 60;
    mBaseGrid.assign(mHeight, std::vector<char>(mWidth, '#'));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 100);

    int numRooms = 10 + (distrib(gen) % 6);
    std::vector<sf::IntRect> rooms;
    rooms.reserve(numRooms);

    for (int i = 0; i < numRooms; i++)
    {
        int attempts = 0;
        bool placed = false;

        while (!placed && attempts < 120)
        {
            int roomWidth = 6 + (distrib(gen) % 7);
            int roomHeight = 6 + (distrib(gen) % 7);
            int roomX = 1 + (distrib(gen) % (mWidth - roomWidth - 2));
            int roomY = 1 + (distrib(gen) % (mHeight - roomHeight - 2));

            sf::IntRect newRoom(roomX, roomY, roomWidth, roomHeight);

            bool intersects = false;
            for (const auto &room : rooms)
            {
                sf::IntRect expandedRoom(
                    room.left - 1,
                    room.top - 1,
                    room.width + 2,
                    room.height + 2);

                if (expandedRoom.intersects(newRoom))
                {
                    intersects = true;
                    break;
                }
            }

            if (!intersects)
            {
                createRoom(roomX, roomY, roomWidth, roomHeight);
                rooms.push_back(newRoom);
                placed = true;
            }

            attempts++;
        }
    }

    if (!rooms.empty())
    {
        auto &startRoom = rooms.front();
        mPlayerStart = sf::Vector2f(
            startRoom.left + startRoom.width / 2.0f,
            startRoom.top + startRoom.height / 2.0f);
    }
    else
    {
        mPlayerStart = sf::Vector2f(mWidth / 2.0f, mHeight / 2.0f);
    }

    if (rooms.size() > 1)
    {
        std::vector<bool> connected(rooms.size(), false);
        connected[0] = true;

        while (std::any_of(connected.begin(), connected.end(), [](bool c)
                           { return !c; }))
        {
            int bestFrom = -1;
            int bestTo = -1;
            int bestDistance = INT_MAX;

            for (size_t i = 0; i < rooms.size(); i++)
            {
                if (!connected[i])
                    continue;

                for (size_t j = 0; j < rooms.size(); j++)
                {
                    if (connected[j] || i == j)
                        continue;

                    int distX = (rooms[i].left + rooms[i].width / 2) - (rooms[j].left + rooms[j].width / 2);
                    int distY = (rooms[i].top + rooms[i].height / 2) - (rooms[j].top + rooms[j].height / 2);
                    int distance = abs(distX) + abs(distY);

                    if (distance < bestDistance)
                    {
                        bestDistance = distance;
                        bestFrom = static_cast<int>(i);
                        bestTo = static_cast<int>(j);
                    }
                }
            }

            if (bestFrom != -1 && bestTo != -1)
            {
                int fromCenterX = rooms[bestFrom].left + rooms[bestFrom].width / 2;
                int fromCenterY = rooms[bestFrom].top + rooms[bestFrom].height / 2;
                int toCenterX = rooms[bestTo].left + rooms[bestTo].width / 2;
                int toCenterY = rooms[bestTo].top + rooms[bestTo].height / 2;

                if (distrib(gen) % 2 == 0)
                {
                    createHorizontalCorridor(fromCenterX, toCenterX, fromCenterY, 3);
                    createVerticalCorridor(toCenterX, fromCenterY, toCenterY, 3);
                }
                else
                {
                    createVerticalCorridor(fromCenterX, fromCenterY, toCenterY, 3);
                    createHorizontalCorridor(fromCenterX, toCenterX, toCenterY, 3);
                }

                connected[bestTo] = true;
            }
        }
    }

    for (int i = 0; i < 8; i++)
    {
        int x = 2 + (distrib(gen) % (mWidth - 6));
        int y = 2 + (distrib(gen) % (mHeight - 6));
        int length = 6 + (distrib(gen) % 14);

        if (distrib(gen) % 2 == 0)
        {
            createHorizontalCorridor(x, x + length, y, 2);
        }
        else
        {
            createVerticalCorridor(x, y, y + length, 2);
        }
    }

    if (!rooms.empty())
    {
        int bestRoomIndex = 0;
        int bestDistance = -1;
        for (size_t i = 0; i < rooms.size(); i++)
        {
            int centerX = rooms[i].left + rooms[i].width / 2;
            int centerY = rooms[i].top + rooms[i].height / 2;
            int distance = abs(centerX - static_cast<int>(mPlayerStart.x)) +
                           abs(centerY - static_cast<int>(mPlayerStart.y));
            if (distance > bestDistance)
            {
                bestDistance = distance;
                bestRoomIndex = static_cast<int>(i);
            }
        }

        int roomCenterX = rooms[bestRoomIndex].left + rooms[bestRoomIndex].width / 2;
        int roomCenterY = rooms[bestRoomIndex].top + rooms[bestRoomIndex].height / 2;

        sf::Vector2i triggerTile(roomCenterX, roomCenterY);
        sf::Vector2f triggerSize(0.9f, 0.35f);

        struct DoorCandidate
        {
            sf::Vector2i wallTile;
            sf::Vector2i triggerTile;
            sf::Vector2f triggerSize;
        };

        std::vector<DoorCandidate> candidates = {
            {{rooms[bestRoomIndex].left - 1, roomCenterY}, {rooms[bestRoomIndex].left, roomCenterY}, sf::Vector2f(0.35f, 0.9f)},
            {{rooms[bestRoomIndex].left + rooms[bestRoomIndex].width, roomCenterY}, {rooms[bestRoomIndex].left + rooms[bestRoomIndex].width - 1, roomCenterY}, sf::Vector2f(0.35f, 0.9f)},
            {{roomCenterX, rooms[bestRoomIndex].top - 1}, {roomCenterX, rooms[bestRoomIndex].top}, sf::Vector2f(0.9f, 0.35f)},
            {{roomCenterX, rooms[bestRoomIndex].top + rooms[bestRoomIndex].height}, {roomCenterX, rooms[bestRoomIndex].top + rooms[bestRoomIndex].height - 1}, sf::Vector2f(0.9f, 0.35f)}};

        for (const auto &candidate : candidates)
        {
            if (candidate.wallTile.x > 0 && candidate.wallTile.x < mWidth - 1 &&
                candidate.wallTile.y > 0 && candidate.wallTile.y < mHeight - 1 &&
                candidate.triggerTile.x > 0 && candidate.triggerTile.x < mWidth - 1 &&
                candidate.triggerTile.y > 0 && candidate.triggerTile.y < mHeight - 1 &&
                mBaseGrid[candidate.wallTile.y][candidate.wallTile.x] == '#' &&
                mBaseGrid[candidate.triggerTile.y][candidate.triggerTile.x] == '.')
            {
                triggerTile = candidate.triggerTile;
                triggerSize = candidate.triggerSize;
                break;
            }
        }

        addTransition(triggerTile.x, triggerTile.y, LevelType::OpenWorld,
                      sf::Vector2f(45.5f, 45.5f), triggerSize);
    }

    for (int y = 1; y < mHeight - 1; y++)
    {
        for (int x = 1; x < mWidth - 1; x++)
        {
            if (mBaseGrid[y][x] == '#')
            {
                int wallNeighbors = 0;
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        if (dx == 0 && dy == 0)
                            continue;
                        if (mBaseGrid[y + dy][x + dx] == '#')
                            wallNeighbors++;
                    }
                }

                if (wallNeighbors == 8)
                {
                    mBaseGrid[y][x] = '.';
                }
            }
        }
    }

    for (int y = 1; y < mHeight - 1; y++)
    {
        for (int x = 1; x < mWidth - 1; x++)
        {
            if (mBaseGrid[y][x] == '.')
            {
                int wallCount = 0;
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        if (dx == 0 && dy == 0)
                            continue;

                        if (mBaseGrid[y + dy][x + dx] == '#')
                            wallCount++;
                    }
                }

                if (wallCount == 8)
                {
                    mBaseGrid[y][x] = '#';
                }
            }
        }
    }

    int safeStartX = std::max(1, std::min(static_cast<int>(mPlayerStart.x), mWidth - 2));
    int safeStartY = std::max(1, std::min(static_cast<int>(mPlayerStart.y), mHeight - 2));
    carveFloorRect(safeStartX - 1, safeStartY - 1, 3, 3);

    int walls = 0, floors = 0;
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            if (mBaseGrid[y][x] == '#')
                walls++;
            else
                floors++;
        }
    }
}

void Map::generateOpenWorldBaseGrid()
{
    mWidth = 90;
    mHeight = 90;
    mBaseGrid.assign(mHeight, std::vector<char>(mWidth, '.'));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 100);

    for (int x = 0; x < mWidth; x++)
    {
        mBaseGrid[0][x] = '#';
        mBaseGrid[mHeight - 1][x] = '#';
    }
    for (int y = 0; y < mHeight; y++)
    {
        mBaseGrid[y][0] = '#';
        mBaseGrid[y][mWidth - 1] = '#';
    }

    int obstacleCount = 20;
    for (int i = 0; i < obstacleCount; i++)
    {
        int blockWidth = 3 + (distrib(gen) % 6);
        int blockHeight = 3 + (distrib(gen) % 6);
        int blockX = 2 + (distrib(gen) % (mWidth - blockWidth - 3));
        int blockY = 2 + (distrib(gen) % (mHeight - blockHeight - 3));

        for (int dy = 0; dy < blockHeight; dy++)
        {
            for (int dx = 0; dx < blockWidth; dx++)
            {
                mBaseGrid[blockY + dy][blockX + dx] = '#';
            }
        }
    }

    int centerX = mWidth / 2;
    int centerY = mHeight / 2;
    carveFloorRect(centerX - 4, centerY - 4, 9, 9);
    mPlayerStart = sf::Vector2f(centerX + 0.5f, centerY + 0.5f);

    std::vector<sf::Vector2i> portalPoints = {
        {10, 10},
        {mWidth - 11, 10},
        {10, mHeight - 11},
        {mWidth - 11, mHeight - 11}};

    for (const auto &point : portalPoints)
    {
        carveFloorRect(point.x - 2, point.y - 2, 5, 5);
        addTransition(point.x, point.y, LevelType::Dungeon, sf::Vector2f(-1.0f, -1.0f));
    }
}

void Map::addTransition(int x, int y, LevelType target, const sf::Vector2f &destinationSpawn,
                        const sf::Vector2f &triggerSize)
{
    sf::FloatRect triggerArea(
        static_cast<float>(x) + 0.5f - triggerSize.x * 0.5f,
        static_cast<float>(y) + 0.5f - triggerSize.y * 0.5f,
        triggerSize.x,
        triggerSize.y);

    mTransitions.push_back({triggerArea, destinationSpawn, target});
    mDoors.push_back({triggerArea, target});
}

void Map::carveFloorRect(int x, int y, int w, int h)
{
    for (int dy = 0; dy < h; dy++)
    {
        for (int dx = 0; dx < w; dx++)
        {
            int px = x + dx;
            int py = y + dy;
            if (px >= 1 && px < mWidth - 1 && py >= 1 && py < mHeight - 1)
            {
                mBaseGrid[py][px] = '.';
            }
        }
    }
}

bool Map::tryCollectAmmoPickup(float x, float y)
{
    for (auto &pickup : mAmmoPickups)
    {
        if (pickup.collected)
        {
            continue;
        }

        float dx = pickup.position.x - x;
        float dy = pickup.position.y - y;
        if (dx * dx + dy * dy <= 0.45f * 0.45f)
        {
            pickup.collected = true;
            return true;
        }
    }

    return false;
}

bool Map::spawnAmmoPickupAt(const sf::Vector2f &position, float minDistance)
{
    if (isWall(position.x, position.y))
    {
        return false;
    }

    float minDistSq = minDistance * minDistance;
    for (const auto &pickup : mAmmoPickups)
    {
        if (pickup.collected)
        {
            continue;
        }

        float dx = pickup.position.x - position.x;
        float dy = pickup.position.y - position.y;
        if (dx * dx + dy * dy < minDistSq)
        {
            return false;
        }
    }

    mAmmoPickups.push_back({position, false});
    return true;
}

void Map::spawnAmmoPickups(int count)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> xDist(1, mWidth - 2);
    std::uniform_int_distribution<> yDist(1, mHeight - 2);

    int spawned = 0;
    int attempts = 0;
    const int maxAttempts = count * 25;

    while (spawned < count && attempts < maxAttempts)
    {
        attempts++;
        float px = static_cast<float>(xDist(gen)) + 0.5f;
        float py = static_cast<float>(yDist(gen)) + 0.5f;

        if (isWall(px, py))
        {
            continue;
        }

        float dxStart = px - mPlayerStart.x;
        float dyStart = py - mPlayerStart.y;
        if (dxStart * dxStart + dyStart * dyStart < 9.0f)
        {
            continue;
        }

        bool overlaps = false;
        for (const auto &pickup : mAmmoPickups)
        {
            float dx = pickup.position.x - px;
            float dy = pickup.position.y - py;
            if (dx * dx + dy * dy < 1.2f * 1.2f)
            {
                overlaps = true;
                break;
            }
        }

        if (overlaps)
        {
            continue;
        }

        mAmmoPickups.push_back({sf::Vector2f(px, py), false});
        spawned++;
    }
}