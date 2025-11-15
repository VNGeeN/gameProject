#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class TextureManager {
    public:
        static TextureManager& getInstance();

        bool loadTexture(const std::string& name, const std::string& filename);
        const sf::Texture& getTexture(const std::string& name) const;

    private:
        TextureManager() = default;

        std::map<std::string, sf::Texture> textures;
};