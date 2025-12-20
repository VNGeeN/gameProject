#include "SkyboxRenderer.h"
#include <iostream>

SkyboxRenderer::SkyboxRenderer(sf::RenderWindow &window)
    : mWindow(window)
{
}

bool SkyboxRenderer::loadFromFile(const std::string &filename)
{
    if (mTexture.loadFromFile(filename))
    {
        mSprite.setTexture(mTexture);
        mIsLoaded = true;
        std::cout << "Skybox loaded: " << filename << " ("
                  << mTexture.getSize().x << "x" << mTexture.getSize().y << ")\n";
        return true;
    }
    else
    {
        std::cerr << "ERROR: Failed to load skybox: " << filename << std::endl;
        return false;
    }
}

void SkyboxRenderer::render(float playerAngle, float horizonY)
{
    if (!mIsLoaded)
        return;

    const int winWidth = mWindow.getSize().x;
    const int winHeight = static_cast<int>(horizonY);

    sf::View originalView = mWindow.getView();
    sf::View skyboxView(sf::FloatRect(0, 0, static_cast<float>(winWidth), horizonY));
    mWindow.setView(skyboxView);

    if (mMode == SkyboxMode::Stretch)
    {
        mSprite.setPosition(0, 0);
        mSprite.setScale(
            static_cast<float>(winWidth) / mTexture.getSize().x,
            static_cast<float>(winHeight) / mTexture.getSize().y);
        mWindow.draw(mSprite);
    }
    else if (mMode == SkyboxMode::Repeat)
    {
        float textureWidth = static_cast<float>(mTexture.getSize().x);
        float offset = -(playerAngle / (2.0f * static_cast<float>(M_PI))) * textureWidth * mParallaxSpeed;
        offset = std::fmod(offset, textureWidth);
        if (offset < 0)
            offset += textureWidth;

        int copies = static_cast<int>(std::ceil(winWidth / textureWidth)) + 2;
        for (int i = 0; i < copies; ++i)
        {
            float x = offset + i * textureWidth;
            mSprite.setPosition(x, 0);
            mSprite.setScale(1.0f, static_cast<float>(winHeight) / mTexture.getSize().y);
            mWindow.draw(mSprite);
        }
    }

    mWindow.setView(originalView);
}