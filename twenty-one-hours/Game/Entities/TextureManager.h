#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

class TextureManager {
public:
    static TextureManager& getInstance() {
        static TextureManager instance;
        return instance;
    }
    
    bool loadAtlas(const std::string& name, const std::string& path);
    const sf::Texture* getAtlas(const std::string& name) const;
    sf::IntRect getRegion(const std::string& material) const;

    void initDefaultRegions();

private:
    TextureManager() = default;
    std::unordered_map<std::string, sf::Texture> mAtlases;
    std::unordered_map<std::string, sf::IntRect> mRegions;
    
};