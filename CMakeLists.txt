cmake_minimum_required( VERSION 3.1.0 )

project( VulkanBoilerPlate
    VERSION 3.1.0
    LANGUAGES CXX
)

set( CMAKE_CXX_STANDARD 17 )

set ( EXTERNAL
    external/glm/glm.hpp
    external/glm/gtx/string_cast.hpp
    external/stb_image/stb_image.h
    external/stb_image/stb_image.cpp
    external/stb_image/stb_image_write.h
    external/stb_image/stb_image_write.cpp
)

set ( INCLUDE
    include/Application.h
    include/utils/Callbacks.h
    include/utils/DebugHelpers.h
    include/utils/Structs.h
    include/utils/Constants.h
    include/utils/FileHandling.h
    include/Window.h
    include/Device.h
    include/SwapChain.h
    include/Buffer.h
    include/descriptors/SetLayout.h
    include/descriptors/Pool.h
    include/descriptors/Set.h
    include/Pipeline.h
    include/Renderer.h
)

set( SOURCE
    src/main.cpp
    src/Application.cpp
    src/Window.cpp
    src/Device.cpp
    src/SwapChain.cpp
    src/Buffer.cpp
    src/descriptors/SetLayout.cpp
    src/descriptors/Pool.cpp
    src/descriptors/Set.cpp
    src/Pipeline.cpp
    src/Renderer.cpp
)

add_executable(VulkanBoilerPlate ${SOURCE} ${INCLUDE} ${EXTERNAL})

# Vulkan linking
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})

# GLFW linking
find_package(glfw3 3.3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)

include_directories(src include external)