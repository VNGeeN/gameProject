#include "TextureManager.h"
#include <iostream>

TextureManager &TextureManager::getInstance()
{
    static TextureManager instance;
    return instance;
}

bool TextureManager::loadTexture(const std::string &name, const std::string &filename)
{
    sf::Texture texture;
    if (!texture.loadFromFile(filename))
    {
        std::cerr << "Failed to load texture: " << filename << std::endl;
    }
    textures[name] = texture;
    std::cout << "Loaded texture: " << name << " from " << filename << std::endl;
    return true;
}

const sf::Texture& TextureManager::getTexture(const std::string& name) const {
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second;
    }
    // Возвращаем пустую текстуру, если не найдена
    static sf::Texture emptyTexture;
    return emptyTexture;
}