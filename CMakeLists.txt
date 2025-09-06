cmake_minimum_required(VERSION 3.20)
project(GlareEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(glare INTERFACE)

# Core dependencies
add_subdirectory(third-party/core/glad EXCLUDE_FROM_ALL)
add_subdirectory(third-party/core/glm EXCLUDE_FROM_ALL)
add_subdirectory(third-party/core/reactphysics3d EXCLUDE_FROM_ALL)
add_subdirectory(third-party/core/tinygltf EXCLUDE_FROM_ALL)

# Include directories for core engine
target_include_directories(glare INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link core dependencies
target_link_libraries(glare INTERFACE 
    glad
    glm
    reactphysics3d
    tinygltf
)

target_compile_features(glare INTERFACE cxx_std_20)

# Build examples
option(GLARE_BUILD_EXAMPLES "Build Glare Engine examples" OFF)
if(GLARE_BUILD_EXAMPLES)
    # Examples dependencies
    add_subdirectory(third-party/examples/glfw EXCLUDE_FROM_ALL)
    add_subdirectory(third-party/examples/imgui EXCLUDE_FROM_ALL)
    
    add_subdirectory(examples)
endif()