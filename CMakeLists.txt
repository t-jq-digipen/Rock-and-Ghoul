# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 3.5)
project(team_aviddivas)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Only include ImportDependencies if the file exists
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/cmake/ImportDependencies.cmake")
    include(ImportDependencies)
    importDependencies()
endif()

add_definitions(-DGLEW_STATIC)
add_definitions(-DUSE_CSD3151_AUTOMATION=0)

set(ALL_LIBS
    glfw
    libglew_static
    imgui
    freetype
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/glm)


# ======================= Auto-discover Source Files =================
# Automatically find all .cpp files in src/
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.c"
)

# Automatically find all header files in include/
file(GLOB_RECURSE HEADERS 
    "include/*.h"
    "include/*.hpp"
)

# Debug output to verify files are found
message(STATUS "Auto-discovered source files: ${SOURCES}")
message(STATUS "Auto-discovered header files: ${HEADERS}")

# ======================= Executable =========================
add_executable(${CMAKE_PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# ======================= Link Libraries =============================
target_link_libraries(${CMAKE_PROJECT_NAME}
    ${ALL_LIBS}
)

# ======================= FMOD Setup =========================
# Use the correct FMOD path structure
set(FMOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/fmod")

# Check if FMOD exists
if(NOT EXISTS "${FMOD_DIR}")
    message(WARNING "FMOD not found at ${FMOD_DIR}. Audio features may not work.")
else()
    message(STATUS "FMOD found at: ${FMOD_DIR}")
endif()

include_directories(${FMOD_DIR}/inc)

# Set architecture-specific lib directory
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64-bit build
    set(FMOD_LIB_DIR "${FMOD_DIR}/lib/x64")
    set(FMOD_LIBRARY_PATH "${FMOD_LIB_DIR}/fmod_vc.lib")
    message(STATUS "Using 64-bit FMOD libraries from: ${FMOD_LIB_DIR}")
else()
    # 32-bit build
    set(FMOD_LIB_DIR "${FMOD_DIR}/lib/x86")
    set(FMOD_LIBRARY_PATH "${FMOD_LIB_DIR}/fmod_vc.lib")
    message(STATUS "Using 32-bit FMOD libraries from: ${FMOD_LIB_DIR}")
endif()

# Verify the library file exists
if(EXISTS "${FMOD_LIBRARY_PATH}")
    message(STATUS "✓ Found fmod_vc.lib at: ${FMOD_LIBRARY_PATH}")
else()
    message(WARNING "✗ fmod_vc.lib NOT found at: ${FMOD_LIBRARY_PATH}")
endif()

# Link FMOD libraries using absolute path
target_link_libraries(${CMAKE_PROJECT_NAME}
    ${ALL_LIBS}
    "${FMOD_LIBRARY_PATH}"
)

# Copy FMOD DLLs on Windows
if(WIN32)
    # Copy main FMOD DLL
    if(EXISTS "${FMOD_LIB_DIR}/fmod.dll")
        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FMOD_LIB_DIR}/fmod.dll
            $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
        )
        message(STATUS "Will copy fmod.dll from: ${FMOD_LIB_DIR}")
    endif()
endif()
# ======================= Set C++ Standard ===========================
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)

# ======================= Compiler Warnings ==========================
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
elseif (MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W3 /WX-)
endif()

# ======================= Include Directories ========================
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ======================= Optional: Organize files in IDE ============
# This creates nice folder structure in Visual Studio/Xcode
source_group("Main" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)
source_group("Engine" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glapp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glapp.h
)
source_group("Audio" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Audio.h
)
source_group("Export" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Export.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Export.h
)
source_group("FrameRateController" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameRateController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FrameRateController.h
)
source_group("Graphics" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Graphics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Render.h
)
source_group("Input" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Input.h
)


source_group("Physics" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Physics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Physics.h
)
source_group("GameStateManager" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameStateManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GameStateManager.h
)
source_group("Camera" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Camera.h
)
source_group("Collision" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Collision.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Collision.h
)

source_group("Transform" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Transform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Transform.h
)

source_group("Sprite" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sprite.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Sprite.h
)

source_group("FilePath" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FilePath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FilePath.h
)

source_group("GameLogic" FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Player.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Player.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Enemy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Enemy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Projectile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Projectile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Spells.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Spells.h
)

