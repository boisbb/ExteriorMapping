cmake_minimum_required( VERSION 3.1.0 )

project( ExteriorMapping
    VERSION 3.1.0
    LANGUAGES CXX
)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR 
    ${CMAKE_CURRENT_BINARY_DIR}/downloaded 
    CACHE PATH "Missing description." FORCE)

set( CMAKE_CXX_STANDARD 17 )

file(GLOB_RECURSE INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/include/utils/*.h)
file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(ExteriorMapping ${SOURCE} ${INCLUDE})

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
find_package(Vulkan COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

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


# Shader compilation code inspired by:
# https://stackoverflow.com/questions/57478571/why-wont-cmake-build-my-vulkan-spirv-shaders
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res/shaders)
set(OUTPUT_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/compiled_shaders)

add_custom_command(
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${OUTPUT_SHADER_DIR}
  OUTPUT ${OUTPUT_SHADER_DIR}
  COMMENT "Creating ${OUTPUT_SHADER_DIR}"
)

file(GLOB SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag ${SHADER_DIR}/*.comp)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
        add_custom_command(
        COMMAND
            glslc 
            -MD -MF ${OUTPUT_SHADER_DIR}/${FILENAME}.d 
            -o ${OUTPUT_SHADER_DIR}/${FILENAME}.spv
            ${SHADER}
            OUTPUT ${OUTPUT_SHADER_DIR}/${FILENAME}.spv
            DEPENDS ${SHADER} ${OUTPUT_SHADER_DIR}
            COMMENT "Compiling ${FILENAME}"
            DEPFILE ${OUTPUT_SHADER_DIR}/${FILENAME}.d 
    )
    list(APPEND SPV_SHADERS ${OUTPUT_SHADER_DIR}/${FILENAME}.spv)
    
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

if(BUILD_DOC)
    find_package(Doxygen REQUIRED)

    set(OUTPUT_DOC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doc)

    add_custom_command(
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DOC_DIR}
    OUTPUT ${OUTPUT_DOC_DIR}
    COMMENT "Creating ${OUTPUT_DOC_DIR}"
    )

    add_custom_target(doc ALL
        COMMAND doxygen ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        DEPENDS ${OUTPUT_DOC_DIR}
    )

    add_dependencies(ExteriorMapping shaders doc)
else()
    add_dependencies(ExteriorMapping shaders)
endif()

target_compile_definitions(ExteriorMapping PRIVATE 
    COMPILED_SHADER_LOC="${OUTPUT_SHADER_DIR}/"
    CONFIG_FILES_LOC="${CMAKE_CURRENT_SOURCE_DIR}/res/configs/"
    SCREENSHOT_FILES_LOC="${CMAKE_CURRENT_SOURCE_DIR}/screenshots/"
    MODELS_FILES_LOC="${CMAKE_CURRENT_SOURCE_DIR}/res/models/"
)