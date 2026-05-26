cmake_minimum_required(VERSION 3.16)

project(GAME VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(GAME src/main.cpp src/app/Game.cpp src/app/GameContext.cpp src/collision/CollisionSystem.cpp src/input/PlayerInput.cpp src/ui/StartScreen.cpp src/entities/Bullet.cpp src/entities/Ship.cpp src/entities/PlayerShip.cpp src/entities/Earth.cpp src/entities/Satellite.cpp src/entities/Asteroid.cpp src/entities/NeutralShip.cpp src/entities/EnemyShip.cpp src/systems/FiringSystem.cpp src/systems/FiringRing.cpp src/systems/SatelliteOrbitSystem.cpp src/systems/SpawnSystem.cpp src/systems/EnemyAISystem.cpp)

target_include_directories(GAME PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(GAME PRIVATE SFML::Graphics SFML::Window SFML::System)

find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
	file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
		"${CMAKE_SOURCE_DIR}/src/*.cpp"
		"${CMAKE_SOURCE_DIR}/src/*.h"
		"${CMAKE_SOURCE_DIR}/include/*.h")
	add_custom_target(format
		COMMAND ${CLANG_FORMAT} -i ${GAME_SOURCES}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Formatting sources with clang-format")
endif()
