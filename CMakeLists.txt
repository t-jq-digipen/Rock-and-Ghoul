# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 3.5)
project(team_aviddivas)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

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

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/cmake/ImportDependencies.cmake")
    include(ImportDependencies)
    importDependencies()
endif()

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
    message(STATUS "Found fmod_vc.lib at: ${FMOD_LIBRARY_PATH}")
else()
    message(WARNING "fmod_vc.lib NOT found at: ${FMOD_LIBRARY_PATH}")
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
# ======================= Mono Embedding Setup =========================
set(MONO_ROOT "${CMAKE_SOURCE_DIR}/deps/Mono")

# --- Headers ---
set(MONO_INCLUDE_DIR "${MONO_ROOT}/include/mono-2.0")
if (EXISTS "${MONO_INCLUDE_DIR}/mono/jit/jit.h")
    message(STATUS "Mono headers found at: ${MONO_INCLUDE_DIR}")
    include_directories(${MONO_INCLUDE_DIR})
else()
    message(WARNING "Mono headers not found! Expected at ${MONO_INCLUDE_DIR}")
endif()

# --- Library ---
set(MONO_LIB "${MONO_ROOT}/lib/mono-2.0-sgen.lib")
if (EXISTS "${MONO_LIB}")
    message(STATUS "Linking Mono: ${MONO_LIB}")
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "${MONO_LIB}")
else()
    message(WARNING "Mono library not found! Expected at ${MONO_LIB}")
endif()

# --- Runtime DLL ---
if (WIN32)
    set(MONO_DLL "${MONO_ROOT}/bin/mono-2.0-sgen.dll")
    if (EXISTS "${MONO_DLL}")
        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying Mono runtime DLL..."
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MONO_DLL}"
                "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/mono-2.0-sgen.dll"
        )
        message(STATUS "Will copy mono-2.0-sgen.dll to build output.")
    else()
        message(WARNING "Mono runtime DLL not found at ${MONO_DLL} — skipping copy.")
    endif()
endif()
# ======================= Copy Mono runtime libraries =========================
set(MONO_LIB_DIR "${MONO_ROOT}/lib/mono/4.5")
set(MONO_DST_DIR "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/mono/lib/mono/4.5")

if (EXISTS "${MONO_LIB_DIR}/mscorlib.dll")
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying Mono runtime libraries..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${MONO_DST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MONO_LIB_DIR}"
            "${MONO_DST_DIR}"
        VERBATIM
    )
    message(STATUS "Will copy Mono runtime libraries from: ${MONO_LIB_DIR}")
else()
    message(WARNING "Mono runtime libraries not found in ${MONO_LIB_DIR}")
endif()
# ======================= Auto-build GameScripts.sln before engine =========================
set(GAMESCRIPTS_SLN "${CMAKE_SOURCE_DIR}/GameScripts/GameScripts.sln")

if (EXISTS "${GAMESCRIPTS_SLN}")
    message(STATUS "Found GameScripts solution at: ${GAMESCRIPTS_SLN}")

    # Optional: auto-detect MSBuild path
find_program(MSBUILD_EXE MSBuild.exe
    HINTS
        "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin"
        "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/BuildTools/MSBuild/Current/Bin"
        "$ENV{ProgramW6432}/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin"
        "$ENV{ProgramW6432}/Microsoft Visual Studio/2022/BuildTools/MSBuild/Current/Bin"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin"
        "C:/Program Files (x86)/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin"
)
    if (NOT MSBUILD_EXE)
        message(WARNING "MSBuild.exe not found — skipping GameScripts build.")
    else()
        add_custom_target(BuildGameScripts ALL
            COMMAND ${CMAKE_COMMAND} -E echo "=== Building GameScripts.sln ==="
            COMMAND "${MSBUILD_EXE}" "${GAMESCRIPTS_SLN}" /t:Build /p:Configuration=$<CONFIG>
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/GameScripts"
            COMMENT "Building GameScripts to produce GameScripts.dll"
            VERBATIM
        )

        # Make sure engine waits for GameScripts.dll
        add_dependencies(${CMAKE_PROJECT_NAME} BuildGameScripts)
    endif()
else()
    message(WARNING "GameScripts.sln not found at ${GAMESCRIPTS_SLN}")
endif()


# ======================= Copy GameScripts DLL =========================
set(GAMESCRIPTS_DLL_PATH "${CMAKE_SOURCE_DIR}/GameScripts/bin/$<CONFIG>/GameScripts.dll")
set(GAMESCRIPTS_TARGET_DIR "${CMAKE_SOURCE_DIR}/Assets/Scripts")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying GameScripts.dll from: ${GAMESCRIPTS_DLL_PATH}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GAMESCRIPTS_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GAMESCRIPTS_DLL_PATH}"
        "${GAMESCRIPTS_TARGET_DIR}/GameScripts.dll"
    VERBATIM
)







# ======================= Set C++ Standard ===========================
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)

# ======================= Compiler Warnings ==========================
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
elseif (MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4 /WX-)
endif()

# ======================= Include Directories ========================
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============== Organize files ============
# This creates nice folder structure in Visual Studio/Xcode

# BASE ENG COMPONENTS

source_group("BaseEngine\\Components" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Camera.h    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glapp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glapp.h
)


source_group("BaseEngine\\CoreEngine" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CoreEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CoreEngine.h
)

source_group("BaseEngine\\Serialization" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FilePath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FilePath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FileIO.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/DataDefs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JsonLoad.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/JsonLoad.h
)


source_group("BaseEngine\\Mono Scripting" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MonoManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MonoManager.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/MonoSystem.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/MonoBindings.cpp
)

source_group("BaseEngine\\CScripting" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/BehaviourFCT.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/BehaviourComponent.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/LogicSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LogicSystem.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/include/GameScripts.h
    
)
source_group("BaseEngine\\Debug" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Debug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CrashHandler.h
)

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

)
source_group("BaseEngine\\ECS\\Component" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Component.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ComponentManager.h
)
source_group("BaseEngine\\ECS\\Component\\Scripting" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MonoScriptComponent.h
)

source_group("BaseEngine\\ECS\\Component\\Graphics" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sprite.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Sprite.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Animator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Animator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/RenderComponent.h

)
source_group("BaseEngine\\ECS\\Component\\Physics" FILES 

    ${CMAKE_CURRENT_SOURCE_DIR}/src/Physics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Physics.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/ForcesManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ForcesManager.h

)

source_group("BaseEngine\\Systems" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/System.h 
)


source_group("BaseEngine\\Systems\\Audio" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AudioSystem.h 
)


source_group("BaseEngine\\Systems\\Render" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/RenderSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderSystem.cpp
)

source_group("BaseEngine\\Systems\\Editor" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImguiSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ImguiSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GuiSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GuiSystem.h
)
source_group("BaseEngine\\Systems\\Physics" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhysicsSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicsSystem.cpp
)

source_group("BaseEngine\\Managers" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameStateManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GameStateManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GameStates.h
)

source_group("BaseEngine\\Managers\\Frame Rate" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameRateController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FrameRateController.h
)

source_group("BaseEngine\\Managers\\Collision" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CollisionManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CollisionManager.h
)



source_group("BaseEngine\\Libraries\\Audio" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Audio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AudioCache.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AudioComponent.h
)


source_group("BaseEngine\\Libraries\\Input" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Input.h

)
source_group("BaseEngine\\Libraries\\Math" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Remath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Remath.h

)
source_group("BaseEngine\\Libraries\\Render" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Render.h

)
source_group("BaseEngine\\Libraries\\Collision" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Collision.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Collision.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SpatialGrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SpatialGrid.h

)

source_group("others" FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Movement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MovementSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Movement.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MovementSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/TextSystem.h
    
)

source_group("others\\EventBus" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EventBus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/EventBus.h
)
source_group("Game\\Levels" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Level1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Level1.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Level2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Level2.h

)

source_group("Game\\Main Menu" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainMenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MainMenu.h
)

source_group("Game\\Main" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

source_group("Game\\Combat System" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CombatSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CombatSystem.cpp
)

source_group("Game\\Game Logic" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PlayerSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlayerSystem.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/Spells.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Spells.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Fretboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Fretboard.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityCreation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/EntityCreation.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/EnemySystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EnemySystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Enemy.h
)

