#include "TextureManager.h"
#include <iostream>

bool TextureManager::loadAtlas(const std::string &name, const std::string &path)
{
    bool ok = mAtlases[name].loadFromFile(path);
    std::cout << "Atlas '" << name << "' load: " << (ok ? "OK" : "FAILED")
              << " from " << path << std::endl;
    return ok;
}

const sf::Texture *TextureManager::getAtlas(const std::string &name) const
{
    auto it = mAtlases.find(name);
    return (it != mAtlases.end()) ? &it->second : nullptr;
}

sf::IntRect TextureManager::getRegion(const std::string &material) const
{
    auto it = mRegions.find(material);
    if (it != mRegions.end())
        return it->second;

    return sf::IntRect(0, 0, 512, 512);
}

void TextureManager::initDefaultRegions()
{
    mRegions["SAND_WALL"] = sf::IntRect(0, 0, 512, 512);
    mRegions["SAND_FLOOR"] = sf::IntRect(512, 0, 512, 512);
    mRegions["STONE_WALL"] = sf::IntRect(1024, 0, 512, 512);
    mRegions["STONE_CEIL"] = sf::IntRect(1536, 0, 512, 512);
}