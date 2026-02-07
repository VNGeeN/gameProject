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

// void Map::initializeBaseGrid()
// {
//     mBaseGrid = {
//         {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
//         {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
//         {'#', '.', '#', '#', '.', '#', '#', '.', '.', '#'},
//         {'#', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
//         {'#', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
//         {'#', '.', '.', '.', '#', '.', '.', '.', '.', '#'},
//         {'#', '.', '#', '.', '#', '.', '#', '.', '.', '#'},
//         {'#', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
//         {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
//         {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}};

//     mWidth = mBaseGrid[0].size();
//     mHeight = mBaseGrid.size();
// }

// void Map::initializeBaseGrid()
// {
//     // Генерируем лабиринт 50x50 с комнатами и коридорами
//     mBaseGrid.clear();
//     mBaseGrid.resize(50, std::vector<char>(50, '#'));

//     // Создаем несколько комнат
//     createRoom(10, 10, 8, 8);
//     createRoom(30, 10, 8, 8);
//     createRoom(10, 30, 8, 8);
//     createRoom(30, 30, 8, 8);

//     // Соединяем комнаты коридорами
//     createHorizontalCorridor(15, 14, 25);
//     createVerticalCorridor(25, 15, 25);

//     mWidth = 50;
//     mHeight = 50;

//     std::cout << "[Map] initializeBaseGrid complete. Size: "
//               << mWidth << "x" << mHeight << std::endl;
// }

void Map::initializeBaseGrid()
{
    mTransitions.clear();

    if (mLevelType == LevelType::OpenWorld)
    {
        generateOpenWorldBaseGrid();
    }
    else
    {
        generateDungeonBaseGrid();
    }
}

// void Map::createRoom(int x, int y, int w, int h)
// {
//     std::cout << "[Map] Creating room at (" << x << "," << y << ") size " << w << "x" << h << std::endl;

//     for (int dy = 0; dy < h; dy++)
//     {
//         for (int dx = 0; dx < w; dx++)
//         {
//             int cellX = x + dx;
//             int cellY = y + dy;

//             // Проверяем границы
//             if (cellX >= 0 && cellX < static_cast<int>(mBaseGrid[0].size()) &&
//                 cellY >= 0 && cellY < static_cast<int>(mBaseGrid.size()))
//             {
//                 mBaseGrid[cellY][cellX] = '.';
//             }
//         }
//     }
// }

void Map::createRoom(int x, int y, int w, int h)
{
    // Гарантируем, что вся комната — пол
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

    // Добавляем немного "неровности" ТОЛЬКО для больших комнат
    if (w > 6 && h > 6)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 100);

        // Делаем 10-20% клеток стенами (только не по краям)
        int centerX = x + w / 2;
        int centerY = y + h / 2;

        for (int dy = 1; dy < h - 1; dy++)
        {
            for (int dx = 1; dx < w - 1; dx++)
            {
                int px = x + dx;
                int py = y + dy;

                // Сохраняем центральную зону комнаты проходимой для надежного спавна/переходов
                if (std::abs(px - centerX) <= 1 && std::abs(py - centerY) <= 1)
                {
                    continue;
                }

                if (distrib(gen) < 15)
                { // 15% шанс стать стеной
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

// void Map::initializeCollisionLayer()
// {
//     for (int y = 0; y < mHeight; y++)
//     {
//         for (int x = 0; x < mWidth; x++)
//         {
//             mCollisionLayer.setCollision(x, y, (mBaseGrid[y][x] == '#'));
//         }
//     }
// }

void Map::initializeCollisionLayer()
{
    // Пересоздаем CollisionLayer с правильными размерами
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
    // Если карта маленькая (< 16x16) - используем старый способ
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

    // Для больших карт - создаем менеджер чанков если нужно
    if (!mChunkManager)
    {
        // const_cast потому что метод константный, но mChunkManager mutable
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

// void Map::initializeSurfaces()
// {
//     int height = mBaseGrid.size();
//     int width = mBaseGrid[0].size();

//     std::cout << "[Map] initializeSurfaces() start" << std::endl;
//     auto &tm = TextureManager::getInstance();
//     const sf::Texture *atlas = tm.getAtlas("main");
//     std::cout << "[Map] Atlas ptr = " << atlas << std::endl;

//     mCells.resize(height);

//     for (int y = 0; y < height; y++)
//     {
//         mCells[y].resize(width);

//         for (int x = 0; x < width; x++)
//         {
//             sf::Vector2f position(static_cast<float>(x), static_cast<float>(y));
//             auto &cell = mCells[y][x];

//             cell.floor = std::make_unique<Surface>(Surface::Type::FLOOR, position);
//             cell.floor->setMaterial(Surface::Material::SAND);
//             if (atlas)
//             {
//                 cell.floor->setTextureAtlas(atlas);
//                 cell.floor->setTextureRegion(tm.getRegion("SAND_FLOOR"));
//             }

//             cell.ceiling = std::make_unique<Surface>(Surface::Type::CEILING, position);
//             cell.ceiling->setMaterial(Surface::Material::STONE);
//             if (atlas)
//             {
//                 cell.ceiling->setTextureAtlas(atlas);
//                 cell.ceiling->setTextureRegion(tm.getRegion("STONE_CEIL"));
//             }

//             if (mBaseGrid[y][x] == '#')
//             {
//                 cell.wall = std::make_unique<Surface>(Surface::Type::WALL, position);
//                 cell.wall->setMaterial(Surface::Material::STONE);
//                 if (atlas)
//                 {
//                     cell.wall->setTextureAtlas(atlas);
//                     cell.wall->setTextureRegion(tm.getRegion("STONE_WALL"));
//                 }
//             }
//         }
//     }
// }

void Map::initializeSurfaces()
{
    // Используем реальные размеры, а не 10x10
    int height = mHeight; // должно быть 50
    int width = mWidth;   // должно быть 50

    std::cout << "[Map] initializeSurfaces() - размеры: "
              << width << "x" << height << std::endl;

    // std::cout << "[Map] First row: ";
    // for (int x = 0; x < std::min(10, width); x++)
    // {
    //     std::cout << mBaseGrid[0][x];
    // }
    // std::cout << "..." << std::endl;

    // std::cout << "[Map] Row 25: ";
    // for (int x = 0; x < std::min(10, width); x++)
    // {
    //     std::cout << mBaseGrid[25][x];
    // }
    // std::cout << "..." << std::endl;

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

            // Создаем пол для ВСЕХ клеток
            cell.floor = std::make_unique<Surface>(Surface::Type::FLOOR, position);
            cell.floor->setMaterial(Surface::Material::SAND);

            // Создаем потолок для ВСЕХ клеток
            cell.ceiling = std::make_unique<Surface>(Surface::Type::CEILING, position);
            cell.ceiling->setMaterial(Surface::Material::STONE);

            // Стена ТОЛЬКО там, где есть '#'
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

            // Текстурируем пол и потолок
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

    // Выводим отладочную информацию
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
}

void Map::generateDungeonBaseGrid()
{
    mWidth = 60;
    mHeight = 60;
    mBaseGrid.assign(mHeight, std::vector<char>(mWidth, '#'));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 100);

    std::cout << "[Map] Generating dungeon level..." << std::endl;

    int numRooms = 10 + (distrib(gen) % 6); // От 10 до 15 комнат
    std::vector<sf::IntRect> rooms;
    rooms.reserve(numRooms);

    for (int i = 0; i < numRooms; i++)
    {
        int attempts = 0;
        bool placed = false;

        while (!placed && attempts < 120)
        {
            int roomWidth = 6 + (distrib(gen) % 7);  // 6-12
            int roomHeight = 6 + (distrib(gen) % 7); // 6-12
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
                std::cout << "[Map] Created room " << i << " at ("
                          << roomX << "," << roomY << ") size "
                          << roomWidth << "x" << roomHeight << std::endl;
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
                std::cout << "[Map] Connected room " << bestFrom << " to room " << bestTo << std::endl;
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
                    std::cout << "[Map] Removed isolated 1x1 at (" << x << "," << y << ")" << std::endl;
                }
            }
        }
    }

    // Гарантируем безопасную стартовую зону после всех пост-обработок генерации
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

    std::cout << "[Map] Dungeon generated: " << walls << " walls, " << floors << " floors" << std::endl;
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

    std::cout << "[Map] Open world generated: " << mWidth << "x" << mHeight << std::endl;
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
    std::cout << "[Map] Transition at (" << x << "," << y << ") -> "
              << (target == LevelType::OpenWorld ? "OpenWorld" : "Dungeon")
              << ", spawn (" << destinationSpawn.x << "," << destinationSpawn.y << ")"
              << std::endl;
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