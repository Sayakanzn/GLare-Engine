cmake_minimum_required(VERSION 3.20)
project(tinygltf)

add_library(tinygltf STATIC
    src/tinygltf.cpp
    include/json.hpp
    include/stb_image.h
    include/stb_image_write.h
    include/tinygltf.h
)

# Add this - tinygltf needs to know where its headers are
target_include_directories(tinygltf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)