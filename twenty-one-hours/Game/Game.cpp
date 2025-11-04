#include "Game.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

Game::Game() 
    : mWindow(sf::VideoMode(800, 600), "twenty-one-hours")
    , mRayCalc(mPlayer, mMap)
    , mRenderer(mWindow, mRayCalc, mMap, mPlayer)
{
    // Можно добавить тестовые объекты на карту
    // mMap.addObject(std::make_unique<GameObject>(sf::Vector2f(5.5f, 5.5f)));
    std::cout << "Game initialized successfully!" << std::endl;
    
}

void Game::run() {
    sf::Clock clock;
    while (mWindow.isOpen()) {
        processEvents();
        sf::Time deltaTime = clock.restart();
        update(deltaTime);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (mWindow.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            mWindow.close();
    }
}

void Game::update(sf::Time deltaTime) {
    float moveSpeed = 3.0f * deltaTime.asSeconds();
    float rotateSpeed = 2.0f * deltaTime.asSeconds();

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
        mPlayer.moveForward(moveSpeed);
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
        mPlayer.moveBackward(moveSpeed);
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
        mPlayer.rotate(-rotateSpeed);
    }
    
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
        mPlayer.rotate(rotateSpeed);
    }
    
    // Пересчитываем лучи каждый кадр
    mRayCalc.calcRays(mWindow.getSize().x);
}

void Game::render() {

    mRenderer.render();

    mWindow.display();
}