#include "Game.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

Game::Game()
    : mWindow(sf::VideoMode(800, 600), "twenty-one-hours")
{
    std::cout << "[Game] Step 1: Loading atlas..." << std::endl;
    auto& tm = TextureManager::getInstance();
    tm.loadAtlas("main", "assets/textures/material_atlas.png");
    tm.initDefaultRegions();

    std::cout << "[Game] Step 2: Creating Map..." << std::endl;
    mMap = std::make_unique<Map>();

    std::cout << "[Game] Step 3: Creating Player..." << std::endl;
    mPlayer = std::make_unique<Player>(*mMap);

    std::cout << "[Game] Step 4: Creating RayCalc..." << std::endl;
    mRayCalc = std::make_unique<RayCalc>(*mPlayer, *mMap);

    std::cout << "[Game] Step 5: Creating Renderer..." << std::endl;
    mRenderer = std::make_unique<Pseudo3DRenderer>(mWindow, *mRayCalc, *mMap, *mPlayer);
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

    mRayCalc->calcRays(mWindow.getSize().x);

    if (mMap->getWidth() > 16 || mMap->getHeight() > 16)
    {
        mMap->updateVisibleChunks(mPlayer->getX(), mPlayer->getY());
    }

    mRayCalc->calcRays(mWindow.getSize().x);
}

void Game::render2D()
{
    mWindow.clear(sf::Color::Black);

    sf::View originalView = mWindow.getView();

    sf::View view(sf::FloatRect(0, 0, 10, 10));
    mWindow.setView(view);

    mMap->drawWalls(mWindow);

    sf::CircleShape playerShape(0.1f);
    playerShape.setFillColor(sf::Color::Green);
    playerShape.setPosition(mPlayer->getX() - 0.1f, mPlayer->getY() - 0.1f);
    mWindow.draw(playerShape);

    sf::VertexArray line(sf::Lines, 2);
    line[0].position = sf::Vector2f(mPlayer->getX(), mPlayer->getY());
    line[0].color = sf::Color::Red;
    line[1].position = sf::Vector2f(
        mPlayer->getX() + cos(mPlayer->getAngle()) * 0.5f,
        mPlayer->getY() + sin(mPlayer->getAngle()) * 0.5f);
    line[1].color = sf::Color::Red;
    mWindow.draw(line);

    mWindow.setView(originalView);
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
    }

    mWindow.display();
}