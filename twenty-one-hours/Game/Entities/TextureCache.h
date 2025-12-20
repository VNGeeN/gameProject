#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class TextureCache {
public:
    static TextureCache& getInstance() {
        static TextureCache instance;
        return instance;
    }

    const sf::Texture* getSandFloor() const { return &m_sandFloor; }
    const sf::Texture* getStoneWall() const { return &m_stoneWall; }
    const sf::Texture* getStoneCeiling() const { return &m_stoneCeiling; }

    bool load() {
        bool ok = true;
        ok &= m_sandFloor.loadFromFile("assets/textures/sand_floor.png");
        ok &= m_stoneWall.loadFromFile("assets/textures/stone_wall.png");
        ok &= m_stoneCeiling.loadFromFile("assets/textures/stone_ceiling.png");
        std::cout << "Sand floor loaded: " 
          << (m_sandFloor.getSize().x > 0) << std::endl;
        return ok;
    }

private:
    TextureCache() = default;
    sf::Texture m_sandFloor;
    sf::Texture m_stoneWall;
    sf::Texture m_stoneCeiling;
};