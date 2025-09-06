add_library(example-common INTERFACE)
target_link_libraries(example-common INTERFACE 
    glare
    glfw
    imgui
)

if(WIN32)
    target_link_libraries(example-common INTERFACE 
        imm32   
        opengl32
        gdi32   
    )
endif()

# Add individual example projects
add_subdirectory(demo-game)
add_subdirectory(physics-demo)
add_subdirectory(lighting-demo)
add_subdirectory(model-viewer-demo)