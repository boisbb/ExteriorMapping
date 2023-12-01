cmake_minimum_required( VERSION 3.1.0 )

project( VulkanBoilerPlate
    VERSION 3.1.0
    LANGUAGES CXX
)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR 
    ${PROJECT_SOURCE_DIR}/external/downloaded 
    CACHE PATH "Missing description." FORCE)

# set(CMAKE_BUILD_TYPE Debug)


set( CMAKE_CXX_STANDARD 17 )

set ( INCLUDE
    include/Application.h
    include/Window.h
    include/Device.h
    include/SwapChain.h
    include/Buffer.h
    include/Pipeline.h
    include/Renderer.h
    include/Scene.h
    include/Model.h
    include/Mesh.h
    include/Camera.h
    include/View.h
    include/Material.h
    include/Image.h
    include/Texture.h
    include/Sampler.h
    include/glm_include_unified.h
    include/descriptors/SetLayout.h
    include/descriptors/Pool.h
    include/descriptors/Set.h
    include/utils/Callbacks.h
    include/utils/DebugHelpers.h
    include/utils/Structs.h
    include/utils/Constants.h
    include/utils/FileHandling.h
    include/utils/Import.h
    include/utils/Input.h
    include/utils/Math.h
)

set( SOURCE
    src/main.cpp
    src/Application.cpp
    src/Window.cpp
    src/Device.cpp
    src/SwapChain.cpp
    src/Buffer.cpp
    src/Camera.cpp
    src/View.cpp
    src/Pipeline.cpp
    src/Renderer.cpp
    src/Scene.cpp
    src/Model.cpp
    src/Mesh.cpp
    src/Material.cpp
    src/Image.cpp
    src/Texture.cpp
    src/Sampler.cpp
    src/descriptors/SetLayout.cpp
    src/descriptors/Pool.cpp
    src/descriptors/Set.cpp
    src/utils/FileHandling.cpp
    src/utils/Import.cpp
    src/utils/Math.cpp
)

add_executable(VulkanBoilerPlate ${SOURCE} ${INCLUDE})

# glfw download and build
FetchContent_Declare(glfw
GIT_REPOSITORY https://github.com/glfw/glfw.git
GIT_TAG master)

set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )

FetchContent_MakeAvailable(glfw)

# assimp download and build
FetchContent_Declare(assimp
GIT_REPOSITORY https://github.com/assimp/assimp.git
GIT_TAG master)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)

# Vulkan linking
find_package(Vulkan REQUIRED)

# ImGui linking
set(IMGUI_PATH "external/imgui/")

file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_include_directories("ImGui" PUBLIC 
    ${IMGUI_PATH}
    ${Vulkan_INCLUDE_DIR}
)
target_link_libraries("ImGui" PUBLIC 
    glfw
    ${Vulkan_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${Vulkan_INCLUDE_DIR}
    ${FETCHCONTENT_BASE_DIR}/glfw-src/include
    ${FETCHCONTENT_BASE_DIR}/assimp-src/include
)

target_link_libraries(${PROJECT_NAME}
    glfw
    assimp
    ${Vulkan_LIBRARIES}
    ImGui
)

# target include dirs
target_include_directories(${PROJECT_NAME} PUBLIC include external)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/
    FILES
    ${SOURCE}
    ${INCLUDE}
)