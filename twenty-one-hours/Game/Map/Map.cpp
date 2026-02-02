#include "Map.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <climits>

Map::Map() : mCollisionLayer(10, 10)
{
    initializeBaseGrid();
    initializeCollisionLayer();
    initializeSurfaces();
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
    mWidth = 50;
    mHeight = 50;
    mBaseGrid.resize(mHeight, std::vector<char>(mWidth, '#'));

    // Используем генератор случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 100);

    std::cout << "[Map] Generating more interesting map..." << std::endl;

    // Создаем 5-8 комнат случайного размера
    int numRooms = 5 + (distrib(gen) % 4); // От 5 до 8 комнат
    std::vector<sf::IntRect> rooms;

    for (int i = 0; i < numRooms; i++)
    {
        int attempts = 0;
        bool placed = false;

        // Пытаемся разместить комнату
        while (!placed && attempts < 100)
        {
            int roomWidth = 5 + (distrib(gen) % 5);  // 4-9
            int roomHeight = 5 + (distrib(gen) % 5); // 4-9
            int roomX = 1 + (distrib(gen) % (mWidth - roomWidth - 2));
            int roomY = 1 + (distrib(gen) % (mHeight - roomHeight - 2));

            sf::IntRect newRoom(roomX, roomY, roomWidth, roomHeight);

            // Проверяем пересечение с существующими комнатами
            bool intersects = false;
            for (const auto &room : rooms)
            {
                // Добавляем зазор в 1 клетку между комнатами
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

    // Соединяем комнаты лабиринтом
    if (rooms.size() > 1)
    {
        std::vector<bool> connected(rooms.size(), false);
        connected[0] = true;

        // Алгоритм Прима для соединения комнат
        while (std::any_of(connected.begin(), connected.end(), [](bool c)
                           { return !c; }))
        {
            int bestFrom = -1;
            int bestTo = -1;
            int bestDistance = INT_MAX;

            // Ищем ближайшую неподключенную комнату к подключенным
            for (size_t i = 0; i < rooms.size(); i++)
            {
                if (!connected[i])
                    continue;

                for (size_t j = 0; j < rooms.size(); j++)
                {
                    if (connected[j] || i == j)
                        continue;

                    // Расстояние между центрами комнат
                    int distX = (rooms[i].left + rooms[i].width / 2) - (rooms[j].left + rooms[j].width / 2);
                    int distY = (rooms[i].top + rooms[i].height / 2) - (rooms[j].top + rooms[j].height / 2);
                    int distance = abs(distX) + abs(distY);

                    if (distance < bestDistance)
                    {
                        bestDistance = distance;
                        bestFrom = i;
                        bestTo = j;
                    }
                }
            }

            if (bestFrom != -1 && bestTo != -1)
            {
                // Соединяем комнаты коридорами
                int fromCenterX = rooms[bestFrom].left + rooms[bestFrom].width / 2;
                int fromCenterY = rooms[bestFrom].top + rooms[bestFrom].height / 2;
                int toCenterX = rooms[bestTo].left + rooms[bestTo].width / 2;
                int toCenterY = rooms[bestTo].top + rooms[bestTo].height / 2;

                // Создаем L-образный коридор
                if (distrib(gen) % 2 == 0)
                {
                    // Сначала горизонтально, затем вертикально
                    createHorizontalCorridor(fromCenterX, toCenterX, fromCenterY);
                    createVerticalCorridor(toCenterX, fromCenterY, toCenterY);
                }
                else
                {
                    // Сначала вертикально, затем горизонтально
                    createVerticalCorridor(fromCenterX, fromCenterY, toCenterY);
                    createHorizontalCorridor(fromCenterX, toCenterX, toCenterY);
                }

                connected[bestTo] = true;
                std::cout << "[Map] Connected room " << bestFrom << " to room " << bestTo << std::endl;
            }
        }
    }

    // Добавляем случайные коридоры для усложнения
    for (int i = 0; i < 5; i++)
    {
        int x = 2 + (distrib(gen) % (mWidth - 4));
        int y = 2 + (distrib(gen) % (mHeight - 4));
        int length = 5 + (distrib(gen) % 10);

        if (distrib(gen) % 2 == 0)
        {
            // Горизонтальный коридор
            createHorizontalCorridor(x, x + length, y);
        }
        else
        {
            // Вертикальный коридор
            createVerticalCorridor(x, y, y + length);
        }
    }

    // Очищаем углы от изолированных стен (опционально)
    for (int y = 1; y < mHeight - 1; y++)
    {
        for (int x = 1; x < mWidth - 1; x++)
        {
            // Если у стены все соседи - тоже стены, делаем ее полом
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

                // Если все 8 соседей - стены, делаем пол
                if (wallNeighbors == 8)
                {
                    mBaseGrid[y][x] = '.';
                }
            }
        }
    }

    // Удаляем изолированные 1x1 проходы
    for (int y = 1; y < mHeight - 1; y++)
    {
        for (int x = 1; x < mWidth - 1; x++)
        {
            if (mBaseGrid[y][x] == '.')
            {
                // Считаем количество соседей-стен
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

                // Если все 8 соседей — стены, это изолированная клетка
                if (wallCount == 8)
                {
                    mBaseGrid[y][x] = '#'; // ← превращаем в стену
                    std::cout << "[Map] Removed isolated 1x1 at (" << x << "," << y << ")" << std::endl;
                }
            }
        }
    }

    // Статистика
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

    std::cout << "[Map] Generated map: " << walls << " walls, " << floors << " floors" << std::endl;
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
        for (int dy = 1; dy < h - 1; dy++)
        {
            for (int dx = 1; dx < w - 1; dx++)
            {
                if (distrib(gen) < 15)
                { // 15% шанс стать стеной
                    mBaseGrid[y + dy][x + dx] = '#';
                }
            }
        }
    }
}

void Map::createHorizontalCorridor(int x1, int x2, int y)
{
    std::cout << "[Map] Creating horizontal corridor from x=" << x1 << " to x=" << x2 << " at y=" << y << std::endl;

    int startX = std::min(x1, x2);
    int endX = std::max(x1, x2);

    for (int x = startX; x <= endX; x++)
    {
        if (x >= 0 && x < static_cast<int>(mBaseGrid[0].size()) &&
            y >= 0 && y < static_cast<int>(mBaseGrid.size()))
        {
            mBaseGrid[y][x] = '.';
        }
    }
}

void Map::createVerticalCorridor(int x, int y1, int y2)
{
    std::cout << "[Map] Creating vertical corridor at x=" << x << " from y=" << y1 << " to y=" << y2 << std::endl;

    int startY = std::min(y1, y2);
    int endY = std::max(y1, y2);

    for (int y = startY; y <= endY; y++)
    {
        if (x >= 0 && x < static_cast<int>(mBaseGrid[0].size()) &&
            y >= 0 && y < static_cast<int>(mBaseGrid.size()))
        {
            mBaseGrid[y][x] = '.';
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
    // Ищем центр первой комнаты
    for (int y = 1; y < mHeight - 1; y++)
    {
        for (int x = 1; x < mWidth - 1; x++)
        {
            if (mBaseGrid[y][x] == '.')
            {
                // Проверяем, достаточно ли места вокруг
                bool valid = true;
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        if (mBaseGrid[y + dy][x + dx] == '#')
                        {
                            valid = false;
                            break;
                        }
                    }
                    if (!valid)
                        break;
                }

                if (valid)
                {
                    return sf::Vector2f(x + 0.5f, y + 0.5f);
                }
            }
        }
    }

    // Если не нашли хорошее место, возвращаем центр карты
    return sf::Vector2f(mWidth / 2.0f, mHeight / 2.0f);
}