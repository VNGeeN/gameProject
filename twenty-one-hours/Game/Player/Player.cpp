#include "Player.h"
#include <cmath>
#include <iostream>

Player::Player(){
    x = 1.5f;
    y = 1.5f;
    angle = 0.0f;
    fov = M_PI / 3.0f;
}

void Player::moveForward(float distance){
    // Временные переменные для новой позиции
    float newX = x + cos(angle) * distance;
    float newY = y + sin(angle) * distance;

    // //проверка колизии с картой
    // x = newX;
    // y = newY;

    // Простая проверка коллизий
    if (newX >= 0.5f && newX <= 9.5f && newY >= 0.5f && newY <= 9.5f) {
        x = newX;
        y = newY;
        std::cout << "Moved to: " << x << ", " << y << std::endl;
    }
}

void Player::moveBackward(float distance){
    float newX = x - cos(angle) * distance;
    float newY = y - sin(angle) * distance;
    
    // // TODO: Добавить проверку коллизий с картой
    // x = newX;
    // y = newY;

    // Простая проверка коллизий
    if (newX >= 0.5f && newX <= 9.5f && newY >= 0.5f && newY <= 9.5f) {
        x = newX;
        y = newY;
        std::cout << "Moved to: " << x << ", " << y << std::endl;
    }
}

void Player::rotate(float angleOffset){
    angle += angleOffset;

    // Нормализуем угол
    if (angle < 0) angle += 2 * M_PI;
    if (angle >= 2 * M_PI) angle -= 2 * M_PI;
    std::cout << "New angle: " << angle << std::endl;
}