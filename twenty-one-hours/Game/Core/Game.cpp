#include "Game.h"
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <iostream>

Game::Game()
    : mWindow(sf::VideoMode(800, 600), "twenty-one-hours")
{
    std::cout << "[Game] Step 1: Loading atlas..." << std::endl;
    auto &tm = TextureManager::getInstance();
    tm.loadAtlas("main", "assets/textures/material_atlas.png");
    tm.initDefaultRegions();

    // Загружаем отдельную текстуру врага
    mEnemyTexture.loadFromFile("assets/textures/enemy.png");

    std::cout << "[Game] Step 2: Creating Map..." << std::endl;
    mMap = std::make_unique<Map>();

    std::cout << "[Game] Map size: " << mMap->getWidth()
              << "x" << mMap->getHeight() << std::endl;

    std::cout << "[Game] Step 3: Creating Player..." << std::endl;
    mPlayer = std::make_unique<Player>(*mMap);

    // Проверка позиции игрока
    std::cout << "[Game] Player at: (" << mPlayer->getX()
              << ", " << mPlayer->getY() << ")" << std::endl;

    std::cout << "[Game] Step 4: Creating RayCalc..." << std::endl;
    mRayCalc = std::make_unique<RayCalc>(*mPlayer, *mMap);

    std::cout << "[Game] Step 5: Creating Renderer..." << std::endl;
    mRenderer = std::make_unique<Pseudo3DRenderer>(mWindow, *mRayCalc, *mMap, *mPlayer, &mEnemyTexture);

    std::cout << "[Game] Step 6: Creating EnemyManager..." << std::endl;
    mEnemyManager = std::make_unique<EnemyManager>(*mMap);

    mEnemyManager->spawnEnemies(5); // 5 врагов для началаf
}

void Game::run()
{
    sf::Clock clock;
    while (mWindow.isOpen())
    {
        processEvents();
        sf::Time deltaTime = clock.restart();
        update(deltaTime);
        render();
    }
}

void Game::processEvents()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            mWindow.close();

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        {
            mWindow.setMouseCursorVisible(true);
        }

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::F1)
            {
                toggleDebugMode();
            }

            if (event.key.code == sf::Keyboard::F2)
            {
                mDebug2DMode = !mDebug2DMode;
                std::cout << "2D Debug mode: " << (mDebug2DMode ? "ON" : "OFF") << std::endl;
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F3)
        {
            toggleChunkDebugMode();
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            mWindow.setMouseCursorVisible(false);
        }
    }

    if (!mWindow.hasFocus())
        return;

    static bool mouseCaptured = true;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
    {
        mouseCaptured = false;
        mWindow.setMouseCursorVisible(true);
    }
    else if (sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
             sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        mouseCaptured = true;
        mWindow.setMouseCursorVisible(false);
    }

    if (!mouseCaptured)
        return;

    sf::Vector2i mousePos = sf::Mouse::getPosition(mWindow);
    sf::Vector2i center(mWindow.getSize().x / 2, mWindow.getSize().y / 2);

    int deltaX = mousePos.x - center.x;
    int deltaY = mousePos.y - center.y;

    if (deltaX != 0 || deltaY != 0)
    {
        float sensitivity = 0.002f;
        mPlayer->rotate(deltaX * sensitivity);

        // Наклон по Y
        // float pitchSensitivity = 0.001f;
        // mCameraPitch += deltaY * pitchSensitivity;
        // mCameraPitch = std::clamp(mCameraPitch, -1.0f, 1.0f);

        sf::Mouse::setPosition(center, mWindow);
    }
}

void Game::toggleDebugMode()
{
    mDebugMode = !mDebugMode;

    for (int y = 0; y < mMap->getHeight(); y++)
    {
        for (int x = 0; x < mMap->getWidth(); x++)
        {
            auto wall = mMap->getWallAt(x, y);
            if (wall)
            {
                wall->setDebugMode(mDebugMode);
            }
        }
    }

    std::cout << "Debug mode: " << (mDebugMode ? "ON" : "OFF") << std::endl;
}

void Game::update(sf::Time deltaTime)
{
    float moveSpeed = 3.0f * deltaTime.asSeconds();
    float rotateSpeed = 2.0f * deltaTime.asSeconds();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        mPlayer->moveForward(moveSpeed);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        mPlayer->moveBackward(moveSpeed);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        mPlayer->rotate(-rotateSpeed);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        mPlayer->rotate(rotateSpeed);
    }

    if (mMap->getWidth() > 16 || mMap->getHeight() > 16)
    {
        mMap->updateVisibleChunks(mPlayer->getX(), mPlayer->getY());
    }

    // Обновляем врагов
    if (mEnemyManager)
    {
        mEnemyManager->update(deltaTime, *mPlayer);
    }

    handleLevelTransitions(deltaTime);

    mRayCalc->calcRays(mWindow.getSize().x);
}

void Game::render2D()
{
    mWindow.clear(sf::Color::Black);

    sf::View originalView = mWindow.getView();

    // Динамический вид на основе размера карты
    float mapWidth = static_cast<float>(mMap->getWidth());
    float mapHeight = static_cast<float>(mMap->getHeight());

    // Вид, показывающий всю карту плюс небольшие отступы
    float padding = 2.0f; // Отступ от краев
    sf::View view(sf::FloatRect(
        -padding,
        -padding,
        mapWidth + 2 * padding,
        mapHeight + 2 * padding));

    // Центрируем вид на середине карты
    view.setCenter(mapWidth / 2.0f, mapHeight / 2.0f);
    mWindow.setView(view);

    // Отладочная информация
    if (mChunkDebugMode)
    {
        // Рисуем сетку (опционально)
        drawGrid();
    }

    // Отрисовка в правильном порядке
    mMap->drawFloors(mWindow);
    mMap->drawWalls(mWindow);

    for (const auto &transition : mMap->getTransitions())
    {
        sf::RectangleShape marker(sf::Vector2f(0.8f, 0.8f));
        marker.setPosition(transition.tile.x + 0.1f, transition.tile.y + 0.1f);
        if (transition.target == Map::LevelType::OpenWorld)
        {
            marker.setFillColor(sf::Color(80, 200, 255));
        }
        else
        {
            marker.setFillColor(sf::Color(200, 120, 255));
        }
        mWindow.draw(marker);
    }

    // Игрок (в мировых координатах)
    sf::CircleShape playerShape(0.2f); // Немного больше для видимости
    playerShape.setFillColor(sf::Color::Green);
    playerShape.setPosition(mPlayer->getX() - 0.1f, mPlayer->getY() - 0.1f);
    mWindow.draw(playerShape);

    // Линия направления взгляда
    sf::VertexArray line(sf::Lines, 2);
    line[0].position = sf::Vector2f(mPlayer->getX(), mPlayer->getY());
    line[0].color = sf::Color::Red;
    line[1].position = sf::Vector2f(
        mPlayer->getX() + cos(mPlayer->getAngle()) * 1.0f,
        mPlayer->getY() + sin(mPlayer->getAngle()) * 1.0f);
    line[1].color = sf::Color::Red;
    mWindow.draw(line);

    if (mEnemyManager)
    {
        mEnemyManager->render2D(mWindow);
    }

    mWindow.setView(originalView);
}

void Game::drawGrid()
{
    // Рисуем сетку для отладки (опционально)
    sf::VertexArray grid(sf::Lines);

    // Вертикальные линии
    for (int x = 0; x <= mMap->getWidth(); x++)
    {
        grid.append(sf::Vertex(sf::Vector2f(static_cast<float>(x), 0.0f), sf::Color(100, 100, 100, 100)));
        grid.append(sf::Vertex(sf::Vector2f(static_cast<float>(x), static_cast<float>(mMap->getHeight())), sf::Color(100, 100, 100, 100)));
    }

    // Горизонтальные линии
    for (int y = 0; y <= mMap->getHeight(); y++)
    {
        grid.append(sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(y)), sf::Color(100, 100, 100, 100)));
        grid.append(sf::Vertex(sf::Vector2f(static_cast<float>(mMap->getWidth()), static_cast<float>(y)), sf::Color(100, 100, 100, 100)));
    }

    mWindow.draw(grid);
}

void Game::render()
{
    mWindow.clear(sf::Color::Black);

    if (mDebug2DMode)
    {
        render2D();
    }
    else
    {
        mRenderer->render();

         // Рисуем врагов в 3D режиме
        if (mEnemyManager)
        {
            mEnemyManager->render3D(*mRenderer, *mPlayer, *mRayCalc);
        }
    }

    mWindow.display();
}

void Game::toggleChunkDebugMode()
{
    mChunkDebugMode = !mChunkDebugMode;
    std::cout << "Chunk debug mode: " << (mChunkDebugMode ? "ON" : "OFF") << std::endl;
}

void Game::handleLevelTransitions(sf::Time deltaTime)
{
    if (mTransitionCooldown > 0.0f)
    {
        mTransitionCooldown = std::max(0.0f, mTransitionCooldown - deltaTime.asSeconds());
        return;
    }

    Map::LevelType target;
    if (!mMap->tryGetTransitionTarget(mPlayer->getX(), mPlayer->getY(), target))
    {
        return;
    }

    std::cout << "[Game] Transitioning to "
              << (target == Map::LevelType::OpenWorld ? "OpenWorld" : "Dungeon")
              << std::endl;

    mMap->regenerate(target);
    sf::Vector2f startPos = mMap->findPlayerStartPosition();
    mPlayer->setPosition(startPos.x, startPos.y);

    mEnemyManager = std::make_unique<EnemyManager>(*mMap);
    mEnemyManager->spawnEnemies(5);

    mTransitionCooldown = 1.0f;
}