#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <queue>
#include <algorithm>
#include "../Entities/GameObject.h"

class Quadtree {
public:
    Quadtree(const sf::FloatRect& bounds, int maxObjects = 8, int maxDepth = 6);
    
    void insert(GameObject* object);
    void remove(GameObject* object);
    void clear();
    void rebuild();
    
    void queryDepthFirst(const sf::Vector2f& center, float radius, 
                        std::vector<GameObject*>& results) const;
    void queryBreadthFirst(const sf::Vector2f& center, float radius,
                          std::vector<GameObject*>& results) const;
    void queryArea(const sf::FloatRect& area, std::vector<GameObject*>& results) const;
    
    void drawDebug(sf::RenderTarget& target, sf::RenderStates states) const;

private:
    struct Node {
        sf::FloatRect bounds;
        std::vector<GameObject*> objects;
        std::array<std::unique_ptr<Node>, 4> children;
        bool isLeaf = true;
        
        Node(const sf::FloatRect& rect) : bounds(rect) {}
    };
    
    void insertRecursive(Node* node, GameObject* object, int depth);
    void queryDepthFirstRecursive(const Node* node, const sf::Vector2f& center, 
                                 float radius, std::vector<GameObject*>& results) const;
    void queryAreaRecursive(const Node* node, const sf::FloatRect& area, 
                           std::vector<GameObject*>& results) const;
    void splitNode(Node* node, int depth);
    void drawDebugRecursive(const Node* node, sf::RenderTarget& target, 
                           sf::RenderStates states) const;
    
    std::unique_ptr<Node> mRoot;
    int mMaxObjects;
    int mMaxDepth;
    std::vector<GameObject*> mAllObjects;
    
    static float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b);
    static float distance(const sf::Vector2f& a, const sf::Vector2f& b);
    static float nodeDistance(const sf::FloatRect& bounds, const sf::Vector2f& point);
};