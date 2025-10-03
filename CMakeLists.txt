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
    nlohmann_json::nlohmann_json
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

if(EXISTS "${FMOD_LIBRARY_PATH}")
    message(STATUS "✓ Found fmod_vc.lib at: ${FMOD_LIBRARY_PATH}")
else()
    message(WARNING "✗ fmod_vc.lib NOT found at: ${FMOD_LIBRARY_PATH}")
endif()

# Link FMOD only if present
if(EXISTS "${FMOD_LIBRARY_PATH}")
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
            ${ALL_LIBS}
            stb
            "${FMOD_LIBRARY_PATH}"
    )
else()
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
            ${ALL_LIBS}
            stb
    )
endif()
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
source_group("BaseEngine\\Editor" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImguiSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ImguiSystem.h
)



# BASE ENG SYSTEMS | FRAMERATE
# =================================================
source_group("BaseEngine\\BaseSystems\\FrameRateController" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameRateController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FrameRateController.h
)
# ================================================= FRAMERATE



# BASE ENG SYSTEMS | AUDIO
# =================================================
source_group("BaseEngine\\BaseSystems\\Audio" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Audio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AudioSystem.h
)
# ================================================= AUDIO

# BASE ENG SYSTEMS

source_group("BaseEngine\\BaseSystems\\Graphics\\Components" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Camera.h    
)

source_group("BaseEngine\\BaseSystems\\Graphics\\Render" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Render.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/RenderSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/RenderSystem.cpp
)
# ================================================= GRAPHICS

# BASE ENG SYSTEMS | INPUT
# =================================================
source_group("BaseEngine\\BaseSystems\\Input" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Input.h
)
# ================================================= INPUT

# BASE ENG SYSTEMS | DEBUG
# =================================================
source_group("BaseEngine\\BaseSystems\\Debug" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Debug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CrashHandler.h
)
# ================================================= DEBUG

# BASE ENG | MATH LIBRARY
# =================================================
source_group("BaseEngine\\Math" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Remath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Remath.h
)
# ================================================= MATH

# GSM
# =================================================
source_group("BaseEngine\\GSM" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameStateManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GameStateManager.h
)
# ================================================= GSM

# ECS
# =================================================
source_group("BaseEngine\\ECS\\Registry" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Registry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Registry.cpp
)
# E
source_group("BaseEngine\\ECS\\Entity" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Entity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/EntityManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityCreation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/EntityCreation.h
)
source_group("BaseEngine\\ECS\\Component" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Component.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ComponentManager.h
)

source_group("BaseEngine\\ECS\\System" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/System.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Registry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Registry.cpp
)

source_group("BaseEngine\\ECS\\Registry" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Registry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Registry.cpp
)

source_group("BaseEngine\\FilePath" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FilePath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FilePath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FileIO.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/DataDefs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JsonLoad.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/JsonLoad.h
)
source_group("BaseEngine\\Serialization" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Serialization.h
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
	${CMAKE_CURRENT_SOURCE_DIR}/src/Animator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Animator.h
)


source_group("Systems\\FrameRateController" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameRateController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FrameRateController.h
)
source_group("Systems\\Graphics\\Render" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Render.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/RenderSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderSystem.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CollisionManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CollisionManager.h
)

source_group("Game\\State\\GameStates" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameStateManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GameStateManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GameStates.h
)
source_group("Game\\State\\Levels" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Level1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Level1.h
)

source_group("Game\\State\\MainMenu" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainMenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MainMenu.h
)
source_group("Game\\GameLogic" FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PlayerSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlayerSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Spells.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Spells.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Fretboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Fretboard.h
)


source_group("Math" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Remath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Remath.h
)

# ======================= Runtime Data (saves folder) =========================
#add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory
#    $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/saves
#)


#file(GLOB SAVE_FILES "${CMAKE_SOURCE_DIR}/saves/*.json")
#foreach(save_file ${SAVE_FILES})
#    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${save_file}
#        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/saves
#    )
#endforeach()

# ======================= Install target =========================

# Install the executable
install(TARGETS ${CMAKE_PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install the saves folder (all json files)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/saves
    DESTINATION bin
    FILES_MATCHING PATTERN "*.json"
)

# Install FMOD DLL (Windows only)
if(WIN32 AND EXISTS "${FMOD_LIB_DIR}/fmod.dll")
    install(FILES "${FMOD_LIB_DIR}/fmod.dll"
        DESTINATION bin
    )
endif()

# Copy project documentation into the build folder (beside .sln)
add_custom_target(copy_docs ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/AvidDivas_ReadMe.txt
        ${CMAKE_BINARY_DIR}/AvidDivas_ReadMe.txt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/projectGuide.pdf
        ${CMAKE_BINARY_DIR}/projectGuide.pdf
)