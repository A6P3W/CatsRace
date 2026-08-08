cmake_minimum_required(VERSION 4.2)

set(VCPKG_MANIFEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/BroccoliEngine/Engine" CACHE PATH
    "Directory containing the BroccoliEngine vcpkg manifest")

project(CatsRaceWorkspace LANGUAGES CXX)

set(BROCCOLI_PROJECT_NAME "CatsRace")
set(BROCCOLI_ENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/BroccoliEngine")
set(BROCCOLI_GAME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/CatsRace")
set(BROCCOLI_OUTPUT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

include("${BROCCOLI_ENGINE_ROOT}/ProjectDefaults.cmake")

add_subdirectory(BroccoliEngine/Engine)
add_subdirectory(CatsRace)
