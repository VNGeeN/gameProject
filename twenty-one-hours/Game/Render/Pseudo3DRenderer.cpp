#include "Pseudo3DRenderer.h"
#include <iostream>

Pseudo3DRenderer::Pseudo3DRenderer(sf::RenderWindow &win, RayCalc &ray, Map &m, Player &p)
    : window(win), rayCalc(ray), map(m), player(p)
{
    loadTextures();
}

void Pseudo3DRenderer::loadTextures()
{
    // Загружаем текстуры через TextureManager
    auto &textureManager = TextureManager::getInstance();

    // Загружаем текстуры
    if (!textureManager.loadTexture("wall", "../assets/wall.jpg"))
    {
        std::cout << "Using default color for walls" << std::endl;
    }

    if (!textureManager.loadTexture("floor", "../assets/floor.jpg"))
    {
        std::cout << "Using default color for floor" << std::endl;
    }

    if (!textureManager.loadTexture("ceiling", "../assets/ceiling.jpg"))
    {
        std::cout << "Using default color for ceiling" << std::endl;
    }

    // Сохраняем ссылки на текстуры
    wallTexture = textureManager.getTexture("wall");
    floorTexture = textureManager.getTexture("floor");
    ceilingTexture = textureManager.getTexture("ceiling");
}

void Pseudo3DRenderer::render()
{
    window.clear(sf::Color::Black);
    renderFloorAndCeiling();
    renderWalls();
    // renderObjects();
}

void Pseudo3DRenderer::renderFloorAndCeiling()
{
    sf::Vector2u windowSize = window.getSize();

    // Рисуем пол (нижняя половина экрана)
    sf::RectangleShape floor(sf::Vector2f(windowSize.x, windowSize.y / 2));
    floor.setPosition(0, windowSize.y / 2);
    floor.setTexture(&floorTexture);
    floor.setTextureRect(sf::IntRect(0, 0, windowSize.x, windowSize.y / 2));
    window.draw(floor);

    // Рисуем потолок (верхняя половина экрана)
    sf::RectangleShape ceiling(sf::Vector2f(windowSize.x, windowSize.y / 2));
    ceiling.setPosition(0, 0);
    ceiling.setTexture(&ceilingTexture);
    ceiling.setTextureRect(sf::IntRect(0, 0, windowSize.x, windowSize.y / 2));
    window.draw(ceiling);
}

// void Pseudo3DRenderer::renderFloorAndCeiling()
// {
//     sf::Vector2u windowSize = window.getSize();

//     // Рисуем пол (нижняя половина экрана)
//     sf::RectangleShape floor(sf::Vector2f(windowSize.x, windowSize.y / 2));
//     floor.setPosition(0, windowSize.y / 2);

//     // Если есть текстура пола, используем ее
//     if (floorTexture.getSize().x > 0 && floorTexture.getSize().y > 0)
//     {
//         floor.setTexture(&floorTexture);
//         // Масштабируем текстуру для повторения
//         floor.setTextureRect(sf::IntRect(0, 0, windowSize.x / 4, windowSize.y / 8));
//     }
//     else
//     {
//         floor.setFillColor(sf::Color(100, 100, 100)); // Серый цвет по умолчанию
//     }

//     window.draw(floor);

//     // Рисуем потолок (верхняя половина экрана)
//     sf::RectangleShape ceiling(sf::Vector2f(windowSize.x, windowSize.y / 2));
//     ceiling.setPosition(0, 0);

//     // Если есть текстура потолка, используем ее
//     if (ceilingTexture.getSize().x > 0 && ceilingTexture.getSize().y > 0)
//     {
//         ceiling.setTexture(&ceilingTexture);
//         // Масштабируем текстуру для повторения
//         ceiling.setTextureRect(sf::IntRect(0, 0, windowSize.x / 4, windowSize.y / 8));
//     }
//     else
//     {
//         ceiling.setFillColor(sf::Color(50, 50, 150)); // Синий цвет по умолчанию
//     }

//     window.draw(ceiling);
// }

void Pseudo3DRenderer::renderWalls()
{
    const auto &rays = rayCalc.getRays();

    // Создаем вертексы для эффективного рендеринга
    sf::VertexArray wallVertices(sf::Quads, rays.size() * 4);

    for (int i = 0; i < rays.size(); i++)
    {
        if (rays[i].hitWall && rays[i].distance < MAX_VIEW_DISTANCE)
        {
            renderWallSlice(i, rays[i], wallVertices, i * 4);
        }
    }

    // Рисуем все стены за один вызов
    window.draw(wallVertices, &wallTexture);
}

// void Pseudo3DRenderer::renderWalls()
// {

//     const auto &rays = rayCalc.getRays();

//     int renderedWalls = 0;
//     int verticalWalls = 0;
//     int horizontalWalls = 0;

//     for (int i = 0; i < rays.size(); i++)
//     {
//         if (rays[i].hitWall && rays[i].distance < MAX_VIEW_DISTANCE)
//         {
//             renderWallSlice(i, rays[i]);
//             renderedWalls++;

//             if (rays[i].side == 0)
//             {
//                 verticalWalls++;
//             }
//             else
//             {
//                 horizontalWalls++;
//             }
//         }
//     }
// }

void Pseudo3DRenderer::renderWallSlice(int column, const RayCalc::Ray &ray, sf::VertexArray &vertices, int vertexIndex)
{
    float wallHeight = calculateWallHeight(ray.distance);
    float brightness = calculateBrightness(ray.distance);

    if (wallHeight > window.getSize().y)
    {
        wallHeight = window.getSize().y;
    }

    float yPos = (window.getSize().y - wallHeight) / 2;

    // Вычисляем координаты текстуры
    int texX = static_cast<int>(ray.wallX * wallTexture.getSize().x);
    texX = std::max(0, std::min(texX, static_cast<int>(wallTexture.getSize().x - 1)));

    // Создаем квад для этой полосы стены
    vertices[vertexIndex].position = sf::Vector2f(column, yPos);
    vertices[vertexIndex + 1].position = sf::Vector2f(column + 1, yPos);
    vertices[vertexIndex + 2].position = sf::Vector2f(column + 1, yPos + wallHeight);
    vertices[vertexIndex + 3].position = sf::Vector2f(column, yPos + wallHeight);

    // Устанавливаем координаты текстуры
    vertices[vertexIndex].texCoords = sf::Vector2f(texX, 0);
    vertices[vertexIndex + 1].texCoords = sf::Vector2f(texX + 1, 0);
    vertices[vertexIndex + 2].texCoords = sf::Vector2f(texX + 1, wallTexture.getSize().y);
    vertices[vertexIndex + 3].texCoords = sf::Vector2f(texX, wallTexture.getSize().y);

    // Применяем затемнение
    sf::Color color(255, 255, 255);
    color.r = static_cast<sf::Uint8>(color.r * brightness);
    color.g = static_cast<sf::Uint8>(color.g * brightness);
    color.b = static_cast<sf::Uint8>(color.b * brightness);

    vertices[vertexIndex].color = color;
    vertices[vertexIndex + 1].color = color;
    vertices[vertexIndex + 2].color = color;
    vertices[vertexIndex + 3].color = color;
}

// void Pseudo3DRenderer::renderWallSlice(int column, const RayCalc::Ray &ray)
// {
//     float wallHeight = calculateWallHeight(ray.distance);
//     float brightness = calculateBrightness(ray.distance);

//     if (wallHeight > window.getSize().y)
//     {
//         wallHeight = window.getSize().y;
//     }

//     float yPos = (window.getSize().y - wallHeight) / 2;

//     // Создаем прямоугольник для стены
//     sf::RectangleShape wallSlice(sf::Vector2f(1, wallHeight));
//     wallSlice.setPosition(column, yPos);

//     // Если есть текстура стены, используем ее
//     if (wallTexture.getSize().x > 0 && wallTexture.getSize().y > 0)
//     {
//         // Вычисляем координаты текстуры
//         int texX = static_cast<int>(ray.wallX * wallTexture.getSize().x);
//         if ((ray.side == 0 && ray.hitX > 0) || (ray.side == 1 && ray.hitY < 0))
//         {
//             texX = wallTexture.getSize().x - texX - 1;
//         }

//         // Создаем текстуру для этого среза стены
//         sf::Texture sliceTexture;
//         sf::IntRect textureRect(texX, 0, 1, wallTexture.getSize().y);

//         // Создаем изображение для среза текстуры
//         sf::Image textureImage = wallTexture.copyToImage();
//         sf::Image sliceImage;
//         sliceImage.create(1, wallTexture.getSize().y);

//         for (unsigned int y = 0; y < wallTexture.getSize().y; y++)
//         {
//             sf::Color pixel = textureImage.getPixel(texX, y);
//             // Применяем затемнение
//             pixel.r = static_cast<sf::Uint8>(pixel.r * brightness);
//             pixel.g = static_cast<sf::Uint8>(pixel.g * brightness);
//             pixel.b = static_cast<sf::Uint8>(pixel.b * brightness);
//             sliceImage.setPixel(0, y, pixel);
//         }

//         sliceTexture.loadFromImage(sliceImage);
//         wallSlice.setTexture(&sliceTexture);

//         // Масштабируем текстуру по высоте стены
//         wallSlice.setTextureRect(sf::IntRect(0, 0, 1, wallTexture.getSize().y));
//     }
//     else
//     {
//         // Используем цвет, если текстура не загружена
//         sf::Color wallColor = getWallColor(ray);
//         wallColor.r = static_cast<sf::Uint8>(wallColor.r * brightness);
//         wallColor.g = static_cast<sf::Uint8>(wallColor.g * brightness);
//         wallColor.b = static_cast<sf::Uint8>(wallColor.b * brightness);
//         wallSlice.setFillColor(wallColor);
//     }

//     window.draw(wallSlice);
// }

// void Pseudo3DRenderer::renderWallSlice(int column, const RayCalc::Ray &ray)
// {
//     float wallHeight = calculateWallHeight(ray.distance);
//     float brightness = calculateBrightness(ray.distance);

//     if (wallHeight > window.getSize().y)
//     {
//         wallHeight = window.getSize().y;
//     }

//     sf::RectangleShape wallSlice(sf::Vector2f(1, wallHeight));
//     float yPos = (window.getSize().y - wallHeight) / 2;
//     wallSlice.setPosition(column, yPos);

//     sf::Color wallColor = getWallColor(ray);

//     //  // Применяем затемнение для расстояния
//     wallColor.r = static_cast<sf::Uint8>(wallColor.r * brightness);
//     wallColor.g = static_cast<sf::Uint8>(wallColor.g * brightness);
//     wallColor.b = static_cast<sf::Uint8>(wallColor.b * brightness);

//     // wallColor.r *= brightness;
//     // wallColor.g *= brightness;
//     // wallColor.b *= brightness;

//     wallSlice.setFillColor(wallColor);

//     window.draw(wallSlice);
// }

void Pseudo3DRenderer::renderObjects()
{
    // auto objects = map.getObjects();

    // // Сортируем объекты по расстоянию (от дальних к ближним)
    // std::sort(objects.begin(), objects.end(),
    //           [this](GameObject *a, GameObject *b)
    //           {
    //               return getDistanceToPlayer(a) > getDistanceToPlayer(b);
    //           });

    // for (auto *obj : objects)
    // {
    //     auto visibility = rayCalc.getObjectVisibility(obj);
    //     if (visibility.isVisible)
    //     {
    //         renderObject(obj, visibility);
    //     }
    // }
}

void Pseudo3DRenderer::renderObject(GameObject *obj, const ObjectVisibilityData &data)
{
    // obj->updateFromRayData(data);
    // const auto &sprite = obj->getSprite();

    // sf::Vector2f screenPos = calculateScreenPosition(obj->getPosition(), data.distance);
    // float scale = calculateObjectScale(data.distance);

    // sf::Sprite renderSprite = sprite;
    // renderSprite.setScale(scale, scale);
    // renderSprite.setPosition(screenPos);

    // sf::Color color = renderSprite.getColor();
    // float brightness = calculateBrightness(data.distance);
    // color.r *= brightness;
    // color.g *= brightness;
    // color.b *= brightness;
    // renderSprite.setColor(color);

    // window.draw(renderSprite);
}

float Pseudo3DRenderer::calculateWallHeight(float distance)
{
    if (distance <= 0)
        return 0;
    float height = window.getSize().y / distance;
    return height;
    // return std::min(height, float(window.getSize().y));
}

float Pseudo3DRenderer::calculateObjectScale(float distance)
{
    return 1.0f / (distance * 0.5f + 0.1f);
}

float Pseudo3DRenderer::calculateBrightness(float distance)
{
    return 1.0f / (1.0f + distance * 0.4f);
}

sf::Vector2f Pseudo3DRenderer::calculateScreenPosition(sf::Vector2f worldPos, float distance)
{
    // Конвертация мировых координат в экранные с учетом перспективы
    float screenX = (worldPos.x - player.getX()) * 100 / distance + window.getSize().x / 2;
    float screenY = (worldPos.y - player.getY()) * 100 / distance + window.getSize().y / 2;
    return sf::Vector2f(screenX, screenY);
}

sf::Color Pseudo3DRenderer::getWallColor(const RayCalc::Ray &ray)
{
    if (ray.side == 0)
    {
        return sf::Color(180, 80, 80); // Тёмно-красный для вертикальных стен
    }
    else
    {
        return sf::Color(80, 80, 180); // Тёмно-синий для горизонтальных стен
    }
}

float Pseudo3DRenderer::getDistanceToPlayer(GameObject *obj)
{
    sf::Vector2f objPos = obj->getPosition();
    float dx = objPos.x - player.getX();
    float dy = objPos.y - player.getY();
    return std::sqrt(dx * dx + dy * dy);
}

// sf::Color Pseudo3DRenderer::getTextureColor(const sf::Texture& texture, float x, float y)
// {
//     if (texture.getSize().x == 0 || texture.getSize().y == 0) {
//         return sf::Color::White;
//     }

//     sf::Image image = texture.copyToImage();
//     int texX = static_cast<int>(x * texture.getSize().x);
//     int texY = static_cast<int>(y * texture.getSize().y);

//     // Обеспечиваем, чтобы координаты были в пределах текстуры
//     texX = std::max(0, std::min(texX, static_cast<int>(texture.getSize().x - 1)));
//     texY = std::max(0, std::min(texY, static_cast<int>(texture.getSize().y - 1)));

//     return image.getPixel(texX, texY);
// }