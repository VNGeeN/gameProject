// #include "Game/Core/Game.h"
// #include <iostream>

// int main()
// {
//     try
//     {
//         Game game;
//         game.run();
//     }
//     catch (const std::exception& e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return -1;
//     }
    
//     return 0;
// }

#include <SFML/Graphics.hpp>
#include "Game/Core/Game.h"

int main() {
    Game game;
    game.run();
    return 0;
}