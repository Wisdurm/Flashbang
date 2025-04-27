cmake_minimum_required(VERSION 3.5)
project(flashbang)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(SDL3_image REQUIRED CONFIG REQUIRED COMPONENTS SDL3_image-shared)
find_package(SDL3_mixer REQUIRED CONFIG REQUIRED COMPONENTS SDL3_mixer-shared)

add_executable(flashbang main.c)
target_link_libraries(flashbang PRIVATE SDL3_image::SDL3_image SDL3::SDL3 SDL3_mixer::SDL3_mixer)
