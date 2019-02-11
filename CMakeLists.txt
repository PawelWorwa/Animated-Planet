cmake_minimum_required(VERSION 3.13)

project(Procedural_Planet VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_BINARY_DIR}")

add_executable(Procedural_Planet
        .gitignore
        LICENSE
        README.md main.cpp)

set(SFML_DIR "${SFML_PATH}")

find_package(SFML 2.5 COMPONENTS graphics audio)

target_link_libraries(Procedural_Planet sfml-graphics sfml-audio)