cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(game-project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(twenty-one-hours 
    main.cpp
    Game/Core/Game.cpp
    # Game/Entities/GameObject.cpp
    Game/Map/Map.cpp
    Game/Map/Chunk.cpp
    Game/Map/ChunkManager.cpp
    Game/Player/Player.cpp
    Game/Rendering/Ray/RayCalc.cpp
    Game/Rendering/Render/Pseudo3DRenderer.cpp
    Game/Entities/SegmentedWall.cpp      
    Game/Entities/SegmentedFloor.cpp     
    Game/Entities/SegmentedCeiling.cpp
    Game/Entities/Surface.cpp
    Game/Entities/TextureManager.cpp   
    Game/Map/CollisionLayer.cpp
    Game/Rendering/Render/Skybox/SkyboxRenderer.cpp
    Game/Entities/Enemy.cpp
    Game/Entities/EnemyManager.cpp
)

target_include_directories(twenty-one-hours PRIVATE 
    Game
    Game/Core
    Game/Entities
    Game/Data
    Game/GameObject
    Game/Map
    Game/Player
    Game/Rendering/Ray
    Game/Rendering/Render
    Game/Rendering/Render/Skybox
)

target_link_libraries(twenty-one-hours PRIVATE 
    sfml-graphics 
    sfml-window 
    sfml-system
)

file(COPY ${CMAKE_SOURCE_DIR}/assets/
     DESTINATION ${CMAKE_BINARY_DIR}/assets)