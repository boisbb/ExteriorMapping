cmake_minimum_required( VERSION 3.1.0 )

project( VulkanBoilerPlate
    VERSION 3.1.0
    LANGUAGES CXX
)

set( CMAKE_CXX_STANDARD 17 )

set ( EXTERNAL
    external/glm/glm.hpp
    external/glm/gtx/string_cast.hpp
)

set ( INCLUDE
    include/Application.h
    include/Window.h
    include/Device.h
    include/SwapChain.h
    include/Buffer.h
    include/Pipeline.h
    include/Renderer.h
    include/Model.h
    include/Mesh.h
    include/Camera.h
    include/Material.h
    include/Image.h
    include/Texture.h
    include/Sampler.h
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
)

set( SOURCE
    src/main.cpp
    src/Application.cpp
    src/Window.cpp
    src/Device.cpp
    src/SwapChain.cpp
    src/Buffer.cpp
    src/Camera.cpp
    src/Pipeline.cpp
    src/Renderer.cpp
    src/Model.cpp
    src/Mesh.cpp
    src/Material.cpp
    src/Image.cpp
    src/Texture.cpp
    src/Sampler.cpp
    src/descriptors/SetLayout.cpp
    src/descriptors/Pool.cpp
    src/descriptors/Set.cpp
)

add_executable(VulkanBoilerPlate ${SOURCE} ${INCLUDE} ${EXTERNAL})

# Vulkan linking
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})

# GLFW linking
find_package(glfw3 3.3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)

# ASSIMP linking
find_package(assimp REQUIRED)
target_link_libraries(${PROJECT_NAME} assimp)

# target include dirs
include_directories(src include external)