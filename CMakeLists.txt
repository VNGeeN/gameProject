# cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
# project(game-project)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# add_executable(twenty-one-hours 
#     main.cpp
#     Game/Game.cpp
#     Game/GameObject/GameObject.cpp
#     Game/Map/Map.cpp
#     Game/Player/Player.cpp
#     Game/Ray/Ray.cpp
#     Game/Render/Pseudo3DRenderer.cpp
# )

# target_include_directories(twenty-one-hours PRIVATE 
#     Game
#     Game/Data
#     Game/GameObject
#     Game/Map
#     Game/Player
#     Game/Ray
#     Game/Render
# )

# target_link_libraries(twenty-one-hours PRIVATE 
#     sfml-graphics 
#     sfml-window 
#     sfml-system
# )