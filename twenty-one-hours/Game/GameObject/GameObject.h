#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include "../Data/ObjectVisibilityData.h"

class GameObject
{
public:
    enum class State
    {
        DEFAULT,      // Не виден, дефолтное состояние
        VISIBLE_FAR,  // Виден далеко
        VISIBLE_MID,  // Виден на средней дистанции
        VISIBLE_NEAR, // Виден близко
        INTERACTABLE  // Достаточно близко для взаимодействия
    };
    GameObject(sf::Vector2f pos, State defaultState = State::DEFAULT);
    void updateFromRayData(const ObjectVisibilityData &data);
    State getState() const;
    sf::Vector2f getPosition() const;
    const sf::Sprite &getSprite() const;
    void setStateSprite(State state, const sf::Texture &texture);

private:
    sf::Vector2f position;
    State currentState;
    State defaultState;

    // Текстуры/спрайты для разных состояний
    std::map<State, sf::Sprite> stateSprites;
};

#endif