#include "Game.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

Game::Game()
    : mWindow(sf::VideoMode(800, 600), "twenty-one-hours")
{
    if (!mUiFont.loadFromFile("assets/fonts/DejaVuSans.ttf"))
    {
        std::cerr << "[Game] Failed to load UI font assets/fonts/DejaVuSans.ttf" << std::endl;
    }

    std::cout << "[Game] Step 1: Loading atlas..." << std::endl;
    auto &tm = TextureManager::getInstance();
    tm.loadAtlas("main", "assets/textures/material_atlas.png");
    tm.initDefaultRegions();

    // Загружаем отдельную текстуру врага
    mEnemyTexture.loadFromFile("assets/textures/enemy.png");

    mWeaponTextureLoaded = mWeaponTexture.loadFromFile("assets/textures/weapon_hands.png");
    if (!mWeaponTextureLoaded)
    {
        std::cerr << "[Game] Failed to load weapon texture assets/textures/weapon_hands.png" << std::endl;
    }
    else
    {
        const int frameWidth = 256;
        const int frameHeight = 128;
        for (int col = 0; col < 4; col++)
        {
            mWeaponIdleFrames.emplace_back(col * frameWidth, 0, frameWidth, frameHeight);
            mWeaponWalkFrames.emplace_back(col * frameWidth, frameHeight, frameWidth, frameHeight);
            mWeaponShootFrames.emplace_back(col * frameWidth, frameHeight * 2, frameWidth, frameHeight);
        }
    }

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

    mEnemyManager->spawnEnemies(6);
    // mEnemyManager->spawnBoss();
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
            if (mState == GameState::Playing)
            {
                mState = GameState::Paused;
                mWindow.setMouseCursorVisible(true);
                mPauseMenuIndex = 0;
            }
            else if (mState == GameState::Paused)
            {
                mState = GameState::Playing;
                mWindow.setMouseCursorVisible(false);
            }
        }

        if (event.type == sf::Event::KeyPressed)
        {
            handleMenuInput(event);
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
            if (mState == GameState::Playing)
            {
                mWindow.setMouseCursorVisible(false);
            }
        }
    }

    if (mState != GameState::Playing)
        return;

    if (!mWindow.hasFocus())
        return;

    static bool mouseCaptured = true;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
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
    if (mState != GameState::Playing)
        return;

    float moveSpeed = 3.0f * deltaTime.asSeconds();
    float rotateSpeed = 2.0f * deltaTime.asSeconds();

    bool isMoving = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        mPlayer->moveForward(moveSpeed);
        isMoving = true;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        mPlayer->moveBackward(moveSpeed);
        isMoving = true;
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

    mPlayer->updateWeaponCooldown(deltaTime);

    updateWeaponAnimation(deltaTime, isMoving);

    if (isMoving)
    {
        mStepIntervalTimer += deltaTime.asSeconds();
        mWeaponBobPhase += deltaTime.asSeconds() * 10.0f;
        if (mStepIntervalTimer >= 0.45f)
        {
            mStepIntervalTimer = 0.0f;
            mStepShakeTimer = 0.12f;
        }
    }
    else
    {
        mStepIntervalTimer = 0.0f;
        mWeaponBobPhase = 0.0f;
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        if (mPlayer->tryFire(deltaTime))
        {
            const auto &weapon = mPlayer->getWeaponStats();
            Enemy *target = nullptr;
            if (mEnemyManager)
            {
                target = mEnemyManager->findTargetInSight(*mPlayer, *mRayCalc,
                                                          weapon.range, weapon.aimCone);
            }

            if (target)
            {
                int bonus = mPlayer->getStats().strength / 2;
                target->takeDamage(weapon.damage + bonus);
                mHitMarkerTimer = 0.15f;
            }

            mWeaponKick = 1.0f;
            mWeaponShootTimer = 0.2f;
            mShotShakeTimer = 0.18f;
        }
    }

    mWeaponKick = std::max(0.0f, mWeaponKick - deltaTime.asSeconds() * 5.0f);
    mHitMarkerTimer = std::max(0.0f, mHitMarkerTimer - deltaTime.asSeconds());
    mWeaponShootTimer = std::max(0.0f, mWeaponShootTimer - deltaTime.asSeconds());
    mShotShakeTimer = std::max(0.0f, mShotShakeTimer - deltaTime.asSeconds());
    mStepShakeTimer = std::max(0.0f, mStepShakeTimer - deltaTime.asSeconds());

    mShakePhase += deltaTime.asSeconds() * 30.0f;
    float shotStrength = (mShotShakeTimer > 0.0f) ? (mShotShakeTimer / 0.18f) * 6.0f : 0.0f;
    float stepStrength = (mStepShakeTimer > 0.0f) ? (mStepShakeTimer / 0.12f) * 3.0f : 0.0f;
    float totalShake = shotStrength + stepStrength;
    mShakeOffset.x = std::sin(mShakePhase * 2.1f) * totalShake;
    mShakeOffset.y = std::cos(mShakePhase * 1.7f) * totalShake;

    handleLevelTransitions(deltaTime);

    if (mEnemyManager && mEnemyManager->hasBoss() && !mEnemyManager->isBossAlive())
    {
        mState = GameState::Victory;
        mWindow.setMouseCursorVisible(true);
    }

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
        marker.setPosition(transition.triggerArea.left, transition.triggerArea.top);
        marker.setSize(sf::Vector2f(transition.triggerArea.width, transition.triggerArea.height));
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

    if (mState == GameState::MainMenu)
    {
        renderMainMenu();
        mWindow.display();
        return;
    }

    if (mState == GameState::Victory)
    {
        renderVictoryScreen();
        mWindow.display();
        return;
    }

    if (mDebug2DMode)
    {
        render2D();
    }
    else
    {
        sf::View view = mWindow.getDefaultView();
        view.move(mShakeOffset);
        mWindow.setView(view);
        mRenderer->render();

        // Рисуем врагов в 3D режиме
        if (mEnemyManager)
        {
            mEnemyManager->render3D(*mRenderer, *mPlayer, *mRayCalc);
        }

        renderHud();
        renderWeapon();
        renderCrosshair();
        renderHitMarker();
        mWindow.setView(mWindow.getDefaultView());
    }

    if (mState == GameState::Paused)
    {
        renderPauseMenu();
    }

    mWindow.display();
}

void Game::renderHud()
{
    const auto &stats = mPlayer->getStats();
    const auto &weapon = mPlayer->getWeaponStats();
    sf::Vector2u size = mWindow.getSize();

    sf::Color panelColor(20, 18, 16, 210);
    sf::Color trimColor(175, 130, 55);
    sf::Color healthColor(170, 30, 30);
    sf::Color armorColor(60, 110, 150);
    sf::Color accentColor(120, 15, 15);

    sf::Vector2f panelSize(280.0f, 96.0f);
    sf::Vector2f panelPos(16.0f, size.y - panelSize.y - 16.0f);

    sf::RectangleShape panel(panelSize);
    panel.setPosition(panelPos);
    panel.setFillColor(panelColor);
    panel.setOutlineThickness(2.0f);
    panel.setOutlineColor(trimColor);
    mWindow.draw(panel);

    sf::RectangleShape header(sf::Vector2f(panelSize.x, 6.0f));
    header.setPosition(panelPos.x, panelPos.y);
    header.setFillColor(accentColor);
    mWindow.draw(header);

    float innerPadding = 12.0f;
    float barWidth = panelSize.x - innerPadding * 2.0f;
    float barHeight = 16.0f;

    sf::RectangleShape healthBack(sf::Vector2f(barWidth, barHeight));
    healthBack.setPosition(panelPos.x + innerPadding, panelPos.y + 24.0f);
    healthBack.setFillColor(sf::Color(40, 10, 10));
    mWindow.draw(healthBack);

    float healthRatio = static_cast<float>(stats.health) / static_cast<float>(stats.maxHealth);
    sf::RectangleShape healthFill(sf::Vector2f(barWidth * healthRatio, barHeight));
    healthFill.setPosition(healthBack.getPosition());
    healthFill.setFillColor(healthColor);
    mWindow.draw(healthFill);

    sf::RectangleShape armorBack(sf::Vector2f(barWidth, barHeight));
    armorBack.setPosition(panelPos.x + innerPadding, panelPos.y + 52.0f);
    armorBack.setFillColor(sf::Color(15, 25, 35));
    mWindow.draw(armorBack);

    float armorRatio = static_cast<float>(stats.armor) / static_cast<float>(stats.maxArmor);
    sf::RectangleShape armorFill(sf::Vector2f(barWidth * armorRatio, barHeight));
    armorFill.setPosition(armorBack.getPosition());
    armorFill.setFillColor(armorColor);
    mWindow.draw(armorFill);

    sf::ConvexShape seal(4);
    seal.setPoint(0, sf::Vector2f(0.0f, 0.0f));
    seal.setPoint(1, sf::Vector2f(14.0f, 6.0f));
    seal.setPoint(2, sf::Vector2f(0.0f, 12.0f));
    seal.setPoint(3, sf::Vector2f(-14.0f, 6.0f));
    seal.setFillColor(trimColor);
    seal.setPosition(panelPos.x + panelSize.x - 28.0f, panelPos.y + panelSize.y - 22.0f);
    mWindow.draw(seal);

    if (mEnemyManager)
    {
        Enemy *target = mEnemyManager->findTargetInSight(*mPlayer, *mRayCalc,
                                                         weapon.range, weapon.aimCone);
        if (target)
        {
            const auto &enemyStats = target->getStats();
            sf::Vector2f targetPanelSize(220.0f, 32.0f);
            sf::Vector2f targetPanelPos((size.x - targetPanelSize.x) * 0.5f, 24.0f);

            sf::RectangleShape targetPanel(targetPanelSize);
            targetPanel.setPosition(targetPanelPos);
            targetPanel.setFillColor(sf::Color(15, 10, 10, 200));
            targetPanel.setOutlineThickness(2.0f);
            targetPanel.setOutlineColor(trimColor);
            mWindow.draw(targetPanel);

            float enemyRatio = static_cast<float>(enemyStats.health) /
                               static_cast<float>(enemyStats.maxHealth);
            sf::RectangleShape enemyBar(sf::Vector2f((targetPanelSize.x - 12.0f) * enemyRatio, 10.0f));
            enemyBar.setPosition(targetPanelPos.x + 6.0f, targetPanelPos.y + 6.0f);
            enemyBar.setFillColor(healthColor);
            mWindow.draw(enemyBar);

            float enemyArmorRatio = static_cast<float>(enemyStats.armor) /
                                    static_cast<float>(enemyStats.maxArmor);
            enemyArmorRatio = std::min(enemyArmorRatio, 1.0f);
            sf::RectangleShape enemyArmor(sf::Vector2f((targetPanelSize.x - 12.0f) * enemyArmorRatio, 6.0f));
            enemyArmor.setPosition(targetPanelPos.x + 6.0f, targetPanelPos.y + 18.0f);
            enemyArmor.setFillColor(armorColor);
            mWindow.draw(enemyArmor);
        }
    }
}

void Game::updateWeaponAnimation(sf::Time deltaTime, bool isMoving)
{
    if (!mWeaponTextureLoaded)
    {
        return;
    }

    WeaponAnimState desiredState = isMoving ? WeaponAnimState::Walk : WeaponAnimState::Idle;
    if (mWeaponShootTimer > 0.0f)
    {
        desiredState = WeaponAnimState::Shoot;
    }

    if (desiredState != mWeaponAnimState)
    {
        mWeaponAnimState = desiredState;
        mWeaponFrameIndex = 0;
        mWeaponFrameTimer = 0.0f;
    }

    const std::vector<sf::IntRect> *frames = &mWeaponIdleFrames;
    float frameDuration = 0.12f;
    if (mWeaponAnimState == WeaponAnimState::Walk && !mWeaponWalkFrames.empty())
    {
        frames = &mWeaponWalkFrames;
        frameDuration = 0.08f;
    }
    else if (mWeaponAnimState == WeaponAnimState::Shoot && !mWeaponShootFrames.empty())
    {
        frames = &mWeaponShootFrames;
        frameDuration = 0.06f;
    }

    if (frames->empty())
    {
        return;
    }

    mWeaponFrameTimer += deltaTime.asSeconds();
    while (mWeaponFrameTimer >= frameDuration)
    {
        mWeaponFrameTimer -= frameDuration;
        mWeaponFrameIndex = (mWeaponFrameIndex + 1) % frames->size();
    }
}

void Game::renderWeapon()
{
    sf::Vector2u size = mWindow.getSize();
    float kick = mWeaponKick * 10.0f;

    float bobX = std::cos(mWeaponBobPhase * 0.7f) * 4.0f;
    float bobY = std::sin(mWeaponBobPhase) * 5.0f;
    sf::Vector2f basePos(size.x * 0.6f + bobX, size.y * 0.95f - kick + bobY);

    if (!mWeaponTextureLoaded || mWeaponIdleFrames.empty())
    {
        sf::RectangleShape body(sf::Vector2f(220.0f, 70.0f));
        body.setPosition(basePos);
        body.setFillColor(sf::Color(35, 32, 30));
        body.setOutlineThickness(2.0f);
        body.setOutlineColor(sf::Color(160, 120, 60));
        mWindow.draw(body);

        sf::RectangleShape barrel(sf::Vector2f(120.0f, 18.0f));
        barrel.setPosition(basePos.x + 180.0f, basePos.y + 22.0f);
        barrel.setFillColor(sf::Color(55, 55, 60));
        barrel.setOutlineThickness(2.0f);
        barrel.setOutlineColor(sf::Color(120, 90, 45));
        mWindow.draw(barrel);

        sf::RectangleShape vent(sf::Vector2f(40.0f, 12.0f));
        vent.setPosition(basePos.x + 24.0f, basePos.y + 10.0f);
        vent.setFillColor(sf::Color(120, 20, 20));
        mWindow.draw(vent);
        return;
    }

    const std::vector<sf::IntRect> *frames = &mWeaponIdleFrames;
    if (mWeaponAnimState == WeaponAnimState::Walk && !mWeaponWalkFrames.empty())
    {
        frames = &mWeaponWalkFrames;
    }
    else if (mWeaponAnimState == WeaponAnimState::Shoot && !mWeaponShootFrames.empty())
    {
        frames = &mWeaponShootFrames;
    }

    std::size_t frameIndex = mWeaponFrameIndex % frames->size();
    sf::Sprite sprite(mWeaponTexture);
    sprite.setTextureRect((*frames)[frameIndex]);
    sprite.setOrigin((*frames)[frameIndex].width / 2.0f, (*frames)[frameIndex].height);

    float scale = 0.9f * (static_cast<float>(size.x) / 800.0f);
    sprite.setScale(scale, scale);
    sprite.setPosition(basePos);
    mWindow.draw(sprite);
}

void Game::renderCrosshair()
{
    sf::Vector2u size = mWindow.getSize();
    float cx = size.x * 0.5f;
    float cy = size.y * 0.5f;

    sf::Color crossColor(200, 170, 90);
    float gap = 6.0f;
    float length = 8.0f;

    sf::VertexArray lines(sf::Lines, 8);
    lines[0].position = sf::Vector2f(cx - gap - length, cy);
    lines[1].position = sf::Vector2f(cx - gap, cy);
    lines[2].position = sf::Vector2f(cx + gap, cy);
    lines[3].position = sf::Vector2f(cx + gap + length, cy);
    lines[4].position = sf::Vector2f(cx, cy - gap - length);
    lines[5].position = sf::Vector2f(cx, cy - gap);
    lines[6].position = sf::Vector2f(cx, cy + gap);
    lines[7].position = sf::Vector2f(cx, cy + gap + length);

    for (std::size_t i = 0; i < 8; i++)
    {
        lines[i].color = crossColor;
    }

    mWindow.draw(lines);
}

void Game::renderHitMarker()
{
    if (mHitMarkerTimer <= 0.0f)
        return;

    sf::Vector2u size = mWindow.getSize();
    float cx = size.x * 0.5f;
    float cy = size.y * 0.5f;
    float len = 10.0f + (0.15f - mHitMarkerTimer) * 40.0f;
    sf::Color color(210, 60, 60, 220);

    sf::VertexArray lines(sf::Lines, 8);
    lines[0].position = sf::Vector2f(cx - len, cy - len);
    lines[1].position = sf::Vector2f(cx - 4.0f, cy - 4.0f);
    lines[2].position = sf::Vector2f(cx + len, cy - len);
    lines[3].position = sf::Vector2f(cx + 4.0f, cy - 4.0f);
    lines[4].position = sf::Vector2f(cx - len, cy + len);
    lines[5].position = sf::Vector2f(cx - 4.0f, cy + 4.0f);
    lines[6].position = sf::Vector2f(cx + len, cy + len);
    lines[7].position = sf::Vector2f(cx + 4.0f, cy + 4.0f);

    for (std::size_t i = 0; i < 8; i++)
    {
        lines[i].color = color;
    }

    mWindow.draw(lines);
}

void Game::handleMenuInput(const sf::Event &event)
{
    if (event.key.code == sf::Keyboard::F1 || event.key.code == sf::Keyboard::F2 ||
        event.key.code == sf::Keyboard::F3)
    {
        return;
    }

    if (mState == GameState::MainMenu)
    {
        const int optionCount = 2;
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
        {
            mMainMenuIndex = (mMainMenuIndex - 1 + optionCount) % optionCount;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
        {
            mMainMenuIndex = (mMainMenuIndex + 1) % optionCount;
        }
        else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
        {
            if (mMainMenuIndex == 0)
            {
                mState = GameState::Playing;
                mWindow.setMouseCursorVisible(false);
            }
            else
            {
                mWindow.close();
            }
        }
        return;
    }

    if (mState == GameState::Victory)
    {
        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
        {
            mState = GameState::MainMenu;
            mMainMenuIndex = 0;
            mWindow.setMouseCursorVisible(true);
        }
        return;
    }

    if (mState == GameState::Paused)
    {
        const int optionCount = 2;
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
        {
            mPauseMenuIndex = (mPauseMenuIndex - 1 + optionCount) % optionCount;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
        {
            mPauseMenuIndex = (mPauseMenuIndex + 1) % optionCount;
        }
        else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
        {
            if (mPauseMenuIndex == 0)
            {
                mState = GameState::Playing;
                mWindow.setMouseCursorVisible(false);
            }
            else
            {
                mState = GameState::MainMenu;
                mWindow.setMouseCursorVisible(true);
                mMainMenuIndex = 0;
            }
        }
    }
}

void Game::renderMainMenu()
{
    sf::Vector2u size = mWindow.getSize();
    sf::RectangleShape background(sf::Vector2f(static_cast<float>(size.x), static_cast<float>(size.y)));
    background.setFillColor(sf::Color(10, 10, 12));
    mWindow.draw(background);

    sf::Text title;
    title.setFont(mUiFont);
    title.setString(L"Двадцать один час");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color(210, 190, 130));
    title.setPosition(size.x * 0.5f - title.getGlobalBounds().width * 0.5f, 80.0f);
    mWindow.draw(title);

    sf::Text subtitle;
    subtitle.setFont(mUiFont);
    subtitle.setString(L"Главное меню");
    subtitle.setCharacterSize(22);
    subtitle.setFillColor(sf::Color(140, 130, 110));
    subtitle.setPosition(size.x * 0.5f - subtitle.getGlobalBounds().width * 0.5f, 140.0f);
    mWindow.draw(subtitle);

    const wchar_t *labels[2] = {L"Начать игру", L"Выход"};

    for (int i = 0; i < 2; ++i)
    {
        bool selected = i == mMainMenuIndex;
        sf::RectangleShape button(sf::Vector2f(320.0f, 52.0f));
        button.setPosition(size.x * 0.5f - 160.0f, 240.0f + i * 70.0f);
        button.setFillColor(selected ? sf::Color(90, 70, 40) : sf::Color(30, 28, 26));
        button.setOutlineThickness(2.0f);
        button.setOutlineColor(selected ? sf::Color(210, 170, 90) : sf::Color(80, 70, 50));
        mWindow.draw(button);

        sf::Text label;
        label.setFont(mUiFont);
        label.setString(labels[i]);
        label.setCharacterSize(24);
        label.setFillColor(selected ? sf::Color(230, 210, 150) : sf::Color(170, 160, 130));
        label.setPosition(button.getPosition().x + 24.0f, button.getPosition().y + 12.0f);
        mWindow.draw(label);
    }

    sf::Text hint;
    hint.setFont(mUiFont);
    hint.setString(L"Управление: стрелки / W,S — выбор, Enter — подтверждение");
    hint.setCharacterSize(16);
    hint.setFillColor(sf::Color(110, 110, 110));
    hint.setPosition(size.x * 0.5f - hint.getGlobalBounds().width * 0.5f, size.y - 60.0f);
    mWindow.draw(hint);
}

void Game::renderVictoryScreen()
{
    sf::Vector2u size = mWindow.getSize();
    sf::RectangleShape background(sf::Vector2f(static_cast<float>(size.x), static_cast<float>(size.y)));
    background.setFillColor(sf::Color(10, 10, 12));
    mWindow.draw(background);

    sf::Text title;
    title.setFont(mUiFont);
    title.setString(L"Победа!");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color(220, 200, 140));
    title.setPosition(size.x * 0.5f - title.getGlobalBounds().width * 0.5f, 120.0f);
    mWindow.draw(title);

    sf::Text subtitle;
    subtitle.setFont(mUiFont);
    subtitle.setString(L"Босс повержен, уровень завершен");
    subtitle.setCharacterSize(22);
    subtitle.setFillColor(sf::Color(150, 140, 120));
    subtitle.setPosition(size.x * 0.5f - subtitle.getGlobalBounds().width * 0.5f, 190.0f);
    mWindow.draw(subtitle);

    sf::Text hint;
    hint.setFont(mUiFont);
    hint.setString(L"Нажмите Enter, чтобы вернуться в меню");
    hint.setCharacterSize(18);
    hint.setFillColor(sf::Color(120, 120, 120));
    hint.setPosition(size.x * 0.5f - hint.getGlobalBounds().width * 0.5f, size.y - 80.0f);
    mWindow.draw(hint);
}

void Game::renderPauseMenu()
{
    sf::Vector2u size = mWindow.getSize();
    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(size.x), static_cast<float>(size.y)));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    mWindow.draw(overlay);

    sf::Text title;
    title.setFont(mUiFont);
    title.setString(L"Пауза");
    title.setCharacterSize(42);
    title.setFillColor(sf::Color(220, 200, 150));
    title.setPosition(size.x * 0.5f - title.getGlobalBounds().width * 0.5f, 90.0f);
    mWindow.draw(title);

    const wchar_t *labels[2] = {L"Продолжить", L"В главное меню"};

    for (int i = 0; i < 2; ++i)
    {
        bool selected = i == mPauseMenuIndex;
        sf::RectangleShape button(sf::Vector2f(320.0f, 52.0f));
        button.setPosition(size.x * 0.5f - 160.0f, 200.0f + i * 70.0f);
        button.setFillColor(selected ? sf::Color(90, 70, 40) : sf::Color(30, 28, 26));
        button.setOutlineThickness(2.0f);
        button.setOutlineColor(selected ? sf::Color(210, 170, 90) : sf::Color(80, 70, 50));
        mWindow.draw(button);

        sf::Text label;
        label.setFont(mUiFont);
        label.setString(labels[i]);
        label.setCharacterSize(24);
        label.setFillColor(selected ? sf::Color(230, 210, 150) : sf::Color(170, 160, 130));
        label.setPosition(button.getPosition().x + 24.0f, button.getPosition().y + 12.0f);
        mWindow.draw(label);
    }

    sf::Text hint;
    hint.setFont(mUiFont);
    hint.setString(L"Esc — продолжить, стрелки / W,S — выбор");
    hint.setCharacterSize(16);
    hint.setFillColor(sf::Color(120, 120, 120));
    hint.setPosition(size.x * 0.5f - hint.getGlobalBounds().width * 0.5f, size.y - 60.0f);
    mWindow.draw(hint);
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
    sf::Vector2f destinationSpawn;
    if (!mMap->tryGetTransitionTarget(mPlayer->getX(), mPlayer->getY(), target, destinationSpawn))
    {
        return;
    }

    std::cout << "[Game] Transitioning to "
              << (target == Map::LevelType::OpenWorld ? "OpenWorld" : "Dungeon")
              << std::endl;

    mMap->regenerate(target);
    
    sf::Vector2f startPos = destinationSpawn;
    bool invalidSpawn =
        startPos.x < 1.0f || startPos.y < 1.0f ||
        startPos.x >= static_cast<float>(mMap->getWidth() - 1) ||
        startPos.y >= static_cast<float>(mMap->getHeight() - 1) ||
        mMap->isWall(startPos.x, startPos.y);

    if (invalidSpawn)
    {
        startPos = mMap->findPlayerStartPosition();
    }

    // Дополнительная защита: если старт в стене (например, после генерации),
    // ищем ближайшую свободную клетку вокруг точки старта.
    if (mMap->isWall(startPos.x, startPos.y))
    {
        bool found = false;
        int baseX = static_cast<int>(startPos.x);
        int baseY = static_cast<int>(startPos.y);
        for (int radius = 1; radius <= 6 && !found; ++radius)
        {
            for (int dy = -radius; dy <= radius && !found; ++dy)
            {
                for (int dx = -radius; dx <= radius && !found; ++dx)
                {
                    int tx = baseX + dx;
                    int ty = baseY + dy;
                    if (tx <= 0 || ty <= 0 || tx >= mMap->getWidth() - 1 || ty >= mMap->getHeight() - 1)
                        continue;

                    float sx = static_cast<float>(tx) + 0.5f;
                    float sy = static_cast<float>(ty) + 0.5f;
                    if (!mMap->isWall(sx, sy))
                    {
                        startPos = sf::Vector2f(sx, sy);
                        found = true;
                    }
                }
            }
        }
    }

    mPlayer->setPosition(startPos.x, startPos.y);

    mEnemyManager = std::make_unique<EnemyManager>(*mMap);
    mEnemyManager->spawnEnemies(6);
    // mEnemyManager->spawnBoss();

    mTransitionCooldown = 1.0f;
}