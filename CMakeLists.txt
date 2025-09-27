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
	stb
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
source_group("BaseEngine\\Engine" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glapp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glapp.h
)

source_group("BaseEngine\\CoreEngine" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CoreEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CoreEngine.h
)

source_group("BaseEngine\\ECS\\Entity" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Entity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/EntityManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityCreation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/EntityCreation.h
)
source_group("BaseEngine\\ECS\\Component" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Component.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ComponentManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ComponentManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Component.cpp
)

source_group("BaseEngine\\ECS\\System" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/System.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SystemManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Registry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Registry.cpp
)

source_group("BaseEngine\\ECS\\Registry" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Registry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Registry.cpp
)


source_group("BaseEngine\\Debug" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DebugTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/DebugTool.h
)
source_group("BaseEngine\\FilePath" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FilePath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FilePath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FileIO.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/DataDefs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/JsonAdapt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JsonLoad.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/JsonLoad.h
)
source_group("BaseEngine\\Serialization" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Serialization.h
)
source_group("BaseEngine\\Transform" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Transform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Transform.h
)

source_group("Systems\\Audio" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Audio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AudioSystem.h
)
source_group("Systems\\Input" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Input.h
)

source_group("Systems\\Graphics\\Components" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Camera.h    
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sprite.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Sprite.h
)


source_group("Systems\\FrameRateController" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameRateController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FrameRateController.h
)
source_group("Systems\\Graphics\\Render" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Render.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/RenderSystem.h
)
source_group("Systems\\Graphics" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Graphics.h
)


source_group("Physics\\System" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Physics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Physics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhysicsSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicsSystem.cpp
)

source_group("Physics\\System\\Collision" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Collision.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Collision.h
)
source_group("Physics\\Movement" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MovementController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MovementController.h
)
source_group("Game\\State\\GameStates" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameStateManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GameStateManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GameStates.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameStates.cpp
)
source_group("Game\\State\\Levels" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Level1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Level2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Level1.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Level2.h
)

source_group("Game\\State\\MainMenu" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainMenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MainMenu.h
)
source_group("Game\\GameLogic" FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PlayerSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlayerSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/BossGhoul.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Projectile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Projectile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Spells.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Spells.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Fretboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Enemy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Enemy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Fretboard.h
)


source_group("Math" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Remath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Remath.h
)
source_group("Export" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Export.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Export.h
)
