cmake_minimum_required(VERSION 3.13)

project(Animated_Planet VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_BINARY_DIR}")

add_executable(Animated_Planet
        .gitignore
        LICENSE
        README.md
        main.cpp
        )

set(SFML_DIR "${SFML_PATH}")

find_package(SFML 2.5 COMPONENTS graphics)

target_link_libraries(Animated_Planet sfml-graphics)