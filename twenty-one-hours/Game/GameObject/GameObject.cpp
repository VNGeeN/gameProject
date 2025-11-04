#include "GameObject.h"

GameObject::GameObject(sf::Vector2f pos, State defaultState)
    : position(pos), defaultState(defaultState), currentState(defaultState) {}

void GameObject::updateFromRayData(const ObjectVisibilityData &data)
{
    if (!data.isVisible)
    {
        currentState = defaultState;
        return;
    }

    // обновляем состояния
    if (data.distance < 1.0f)
    {
        currentState = State::INTERACTABLE;
    }
    else if (data.distance < 3.0f)
    {
        currentState = State::VISIBLE_NEAR;
    }
    else if (data.distance < 7.0f)
    {
        currentState = State::VISIBLE_MID;
    }
    else
    {
        currentState = State::VISIBLE_FAR;
    }
}

GameObject::State GameObject::getState() const
{
    return currentState;
}

sf::Vector2f GameObject::getPosition() const
{
    return position;
}

const sf::Sprite &GameObject::getSprite() const
{
    return stateSprites.at(currentState);
}

void GameObject::setStateSprite(State state, const sf::Texture &texture)
{
    stateSprites[state].setTexture(texture);

    // настройки origin, scale
}
