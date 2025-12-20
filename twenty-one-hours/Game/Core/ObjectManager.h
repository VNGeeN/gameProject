// #pragma once
// #include <memory>
// #include <vector>
// #include <SFML/Graphics.hpp>
// #include "Quadtree.h"
// #include "../Entities/GameObject.h"

// class ObjectManager {
// public:
//     ObjectManager(const sf::FloatRect& worldBounds);
    
//     // Управление объектами
//     void addObject(std::unique_ptr<GameObject> object);
//     void removeObject(GameObject* object);
//     void update(float deltaTime);
//     void clear();
    
//     // Пространственные запросы
//     std::vector<GameObject*> getObjectsForRendering(const sf::Vector2f& viewCenter, 
//                                                    float renderDistance) const;
//     std::vector<GameObject*> getObjectsForCollision(const sf::Vector2f& center, 
//                                                    float checkRadius) const;
//     std::vector<GameObject*> getObjectsInArea(const sf::FloatRect& area) const;
    
//     // Получение всех объектов (для обратной совместимости)
//     std::vector<GameObject*> getAllObjects() const;
    
//     // Отладочная визуализация
//     void drawDebug(sf::RenderTarget& target) const;

// private:
//     std::vector<std::unique_ptr<GameObject>> mObjects;
//     std::unique_ptr<Quadtree> mSpatialIndex;
//     sf::FloatRect mWorldBounds;
    
//     void rebuildSpatialIndex();
// };

// #pragma once
// #include <SFML/Graphics.hpp>
// #include <vector>
// #include <memory>

// class GameObject;

// class ObjectManager {
// public:
//     ObjectManager(const sf::FloatRect& worldBounds) {}
//     void addObject(std::unique_ptr<GameObject> object) {}
//     void removeObject(GameObject* object) {}
//     void update(float deltaTime) {}
//     void clear() {}
    
//     std::vector<GameObject*> getObjectsForRendering(const sf::Vector2f& viewCenter, 
//                                                    float renderDistance) const { return {}; }
//     std::vector<GameObject*> getObjectsForCollision(const sf::Vector2f& center, 
//                                                    float checkRadius) const { return {}; }
//     std::vector<GameObject*> getObjectsInArea(const sf::FloatRect& area) const { return {}; }
//     std::vector<GameObject*> getAllObjects() const { return {}; }
// };

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "GameObject.h"

class ObjectManager {
public:
    ObjectManager(const sf::FloatRect& worldBounds) {
        // заглушка
    }
    
    void addObject(std::unique_ptr<GameObject> object) {
        mObjects.push_back(std::move(object));
    }
    
    void removeObject(GameObject* object) {
        mObjects.erase(std::remove_if(mObjects.begin(), mObjects.end(),
            [object](const std::unique_ptr<GameObject>& obj) {
                return obj.get() == object;
            }), mObjects.end());
    }
    
    void update(float deltaTime) {
        for (auto& obj : mObjects) {
            obj->update(deltaTime);
        }
    }
    
    void clear() {
        mObjects.clear();
    }
    
    std::vector<GameObject*> getObjectsForRendering(const sf::Vector2f& viewCenter, 
                                                   float renderDistance) const {
        std::vector<GameObject*> result;
        for (const auto& obj : mObjects) {
            sf::Vector2f pos = obj->getPosition();
            float dx = pos.x - viewCenter.x;
            float dy = pos.y - viewCenter.y;
            if (dx*dx + dy*dy <= renderDistance * renderDistance) {
                result.push_back(obj.get());
            }
        }
        return result;
    }
    
    std::vector<GameObject*> getObjectsForCollision(const sf::Vector2f& center, 
                                                   float checkRadius) const {
        std::vector<GameObject*> result;
        for (const auto& obj : mObjects) {
            sf::Vector2f pos = obj->getPosition();
            float dx = pos.x - center.x;
            float dy = pos.y - center.y;
            if (dx*dx + dy*dy <= checkRadius * checkRadius) {
                result.push_back(obj.get());
            }
        }
        return result;
    }
    
    std::vector<GameObject*> getObjectsInArea(const sf::FloatRect& area) const {
        std::vector<GameObject*> result;
        for (const auto& obj : mObjects) {
            if (area.contains(obj->getPosition())) {
                result.push_back(obj.get());
            }
        }
        return result;
    }
    
    std::vector<GameObject*> getAllObjects() const {
        std::vector<GameObject*> result;
        for (const auto& obj : mObjects) {
            result.push_back(obj.get());
        }
        return result;
    }

private:
    std::vector<std::unique_ptr<GameObject>> mObjects;
};