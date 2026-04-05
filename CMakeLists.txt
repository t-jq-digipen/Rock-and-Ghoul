# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 3.5)
project(team_aviddivas)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_definitions(-DGLEW_STATIC)
set(ALL_LIBS
    glfw
    libglew_static
    imgui
    imguizmo
    freetype
    nlohmann_json::nlohmann_json
    rectpack2d
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/glm)


# ======================= Auto-discover Game Directory =========================
set(GAME_NAME "" CACHE STRING "Game folder to build (e.g. RockAndGhoul). Leave empty to auto-detect.")

set(EXCLUDED_DIRS "CoreEngine" "deps" "cmake" "build" "out" "GameScripts")
file(GLOB TOP_LEVEL_DIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_LIST_DIR}/*")
set(GAME_DIRS "")
foreach(DIR ${TOP_LEVEL_DIRS})
    if(IS_DIRECTORY "${DIR}" AND IS_DIRECTORY "${DIR}/src")
        get_filename_component(DIR_NAME "${DIR}" NAME)
        if(NOT DIR_NAME IN_LIST EXCLUDED_DIRS)
            list(APPEND GAME_DIRS "${DIR}")
            message(STATUS "Found game directory: ${DIR}")
        endif()
    endif()
endforeach()

if(NOT GAME_DIRS)
    message(FATAL_ERROR "No game directory found! Create a folder with a src/ subdirectory.")
endif()

# If GAME_NAME is specified, use it. Otherwise use the first found.
if(GAME_NAME)
    set(PRIMARY_GAME_DIR "${CMAKE_CURRENT_LIST_DIR}/${GAME_NAME}")
    if(NOT IS_DIRECTORY "${PRIMARY_GAME_DIR}/src")
        message(FATAL_ERROR "GAME_NAME='${GAME_NAME}' not found or has no src/ subdirectory.")
    endif()
    message(STATUS "Using specified game: ${PRIMARY_GAME_DIR}")
else()
    list(LENGTH GAME_DIRS GAME_DIR_COUNT)
    if(GAME_DIR_COUNT GREATER 1)
        message(WARNING
            "Multiple game directories found — building the first one.\n"
            "To pick a specific game run: cmake -DGAME_NAME=<folder> ...")
    endif()
    list(GET GAME_DIRS 0 PRIMARY_GAME_DIR)
    message(STATUS "Primary game directory: ${PRIMARY_GAME_DIR}")
endif()


# ======================= Import Dependencies =========================
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/cmake/ImportDependencies.cmake")
    include(ImportDependencies)
    importDependencies()
endif()


# ======================= FMOD Setup =========================
set(FMOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/fmod")

if(NOT EXISTS "${FMOD_DIR}")
    message(WARNING "FMOD not found at ${FMOD_DIR}. Audio features may not work.")
else()
    message(STATUS "FMOD found at: ${FMOD_DIR}")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FMOD_LIB_DIR "${FMOD_DIR}/lib/x64")
    message(STATUS "Using 64-bit FMOD libraries from: ${FMOD_LIB_DIR}")
else()
    set(FMOD_LIB_DIR "${FMOD_DIR}/lib/x86")
    message(STATUS "Using 32-bit FMOD libraries from: ${FMOD_LIB_DIR}")
endif()

set(FMOD_LIBRARY_PATH "${FMOD_LIB_DIR}/fmod_vc.lib")
if(EXISTS "${FMOD_LIBRARY_PATH}")
    message(STATUS "Found fmod_vc.lib at: ${FMOD_LIBRARY_PATH}")
else()
    message(WARNING "fmod_vc.lib NOT found at: ${FMOD_LIBRARY_PATH}")
endif()


# ======================= Mono Setup =========================
set(MONO_ROOT "${CMAKE_SOURCE_DIR}/deps/Mono")
set(MONO_INCLUDE_DIR "${MONO_ROOT}/include/mono-2.0")
set(MONO_LIB "${MONO_ROOT}/lib/mono-2.0-sgen.lib")

if(EXISTS "${MONO_INCLUDE_DIR}/mono/jit/jit.h")
    message(STATUS "Mono headers found at: ${MONO_INCLUDE_DIR}")
else()
    message(WARNING "Mono headers not found! Expected at ${MONO_INCLUDE_DIR}")
endif()

if(EXISTS "${MONO_LIB}")
    message(STATUS "Linking Mono: ${MONO_LIB}")
else()
    message(WARNING "Mono library not found! Expected at ${MONO_LIB}")
endif()


# =========================
# FFmpeg
# =========================
set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/deps/FFmpeg")

add_library(ffmpeg INTERFACE)
target_include_directories(ffmpeg INTERFACE "${FFMPEG_DIR}/include")
target_link_libraries(ffmpeg INTERFACE
    "${FFMPEG_DIR}/lib/avcodec.lib"
    "${FFMPEG_DIR}/lib/avformat.lib"
    "${FFMPEG_DIR}/lib/avutil.lib"
    "${FFMPEG_DIR}/lib/swscale.lib"
)


# ======================= CoreEngine Library =========================
file(GLOB_RECURSE CE_SOURCES
    "CoreEngine/src/*.cpp"
    "CoreEngine/src/*.c"
)
file(GLOB_RECURSE CE_HEADERS
    "CoreEngine/include/*.h"
    "CoreEngine/include/*.hpp"
)
message(STATUS "CoreEngine source files: ${CE_SOURCES}")
message(STATUS "CoreEngine header files: ${CE_HEADERS}")

add_library(CoreEngine STATIC ${CE_SOURCES} ${CE_HEADERS})

target_include_directories(CoreEngine
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/CoreEngine/include
            ${FMOD_DIR}/inc
            ${MONO_INCLUDE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/CoreEngine/src
            "${PRIMARY_GAME_DIR}/include"
            "${PRIMARY_GAME_DIR}/src"
)

if(EXISTS "${FMOD_LIBRARY_PATH}")
    target_link_libraries(CoreEngine PUBLIC ${ALL_LIBS} stb "${FMOD_LIBRARY_PATH}")
else()
    target_link_libraries(CoreEngine PUBLIC ${ALL_LIBS} stb)
endif()

if(EXISTS "${MONO_LIB}")
    target_link_libraries(CoreEngine PUBLIC "${MONO_LIB}")
endif()

target_link_libraries(CoreEngine PUBLIC ffmpeg)

set_property(TARGET CoreEngine PROPERTY CXX_STANDARD 20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(CoreEngine PRIVATE -Wall)
elseif(MSVC)
    target_compile_options(CoreEngine PRIVATE /W4 /WX-)
endif()


# ======================= Game Sources =========================
file(GLOB_RECURSE GAME_SOURCES
    "${PRIMARY_GAME_DIR}/src/*.cpp"
    "${PRIMARY_GAME_DIR}/src/*.c"
)
file(GLOB_RECURSE GAME_HEADERS
    "${PRIMARY_GAME_DIR}/include/*.h"
    "${PRIMARY_GAME_DIR}/include/*.hpp"
)
message(STATUS "Game source files: ${GAME_SOURCES}")
message(STATUS "Game header files: ${GAME_HEADERS}")


# ======================= Game Executable =========================
add_executable(${CMAKE_PROJECT_NAME} ${GAME_SOURCES} ${GAME_HEADERS})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE CoreEngine)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${PRIMARY_GAME_DIR}/include"
    "${PRIMARY_GAME_DIR}/src"
)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
elseif(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4 /WX-)
endif()


# ======================= Release: Hide Win32 Console =========================
# Switches subsystem to Windows in Release so no cmd window appears
if(WIN32)
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
    )
endif()


# ======================= Post-build: Copy DLLs =========================
# FMOD DLL
if(WIN32 AND EXISTS "${FMOD_LIB_DIR}/fmod.dll")
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FMOD_LIB_DIR}/fmod.dll
            $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
    )
    message(STATUS "Will copy fmod.dll from: ${FMOD_LIB_DIR}")
endif()

# Mono runtime DLL
if(WIN32)
    set(MONO_DLL "${MONO_ROOT}/bin/mono-2.0-sgen.dll")
    if(EXISTS "${MONO_DLL}")
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

# Mono runtime libraries
set(MONO_LIB_DIR "${MONO_ROOT}/lib/mono/4.5")
set(MONO_DST_DIR "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/mono/lib/mono/4.5")
if(EXISTS "${MONO_LIB_DIR}/mscorlib.dll")
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying Mono runtime libraries..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${MONO_DST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${MONO_LIB_DIR}" "${MONO_DST_DIR}"
        VERBATIM
    )
    message(STATUS "Will copy Mono runtime libraries from: ${MONO_LIB_DIR}")
else()
    message(WARNING "Mono runtime libraries not found in ${MONO_LIB_DIR}")
endif()

# FFmpeg DLLs
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${FFMPEG_DIR}/bin"
        "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>"
)


# ======================= GameScripts =========================
set(GAMESCRIPTS_SLN "${CMAKE_SOURCE_DIR}/GameScripts/GameScripts.sln")

if(EXISTS "${GAMESCRIPTS_SLN}")
    message(STATUS "Found GameScripts solution at: ${GAMESCRIPTS_SLN}")

    find_program(MSBUILD_EXE MSBuild.exe
        HINTS
            "$ENV{ProgramFiles}/Microsoft Visual Studio/*/Community/MSBuild/Current/Bin"
            "$ENV{ProgramFiles}/Microsoft Visual Studio/*/BuildTools/MSBuild/Current/Bin"
            "$ENV{ProgramW6432}/Microsoft Visual Studio/*/Community/MSBuild/Current/Bin"
            "$ENV{ProgramW6432}/Microsoft Visual Studio/*/BuildTools/MSBuild/Current/Bin"
            "C:/Program Files/Microsoft Visual Studio/*/Community/MSBuild/Current/Bin"
            "C:/Program Files (x86)/Microsoft Visual Studio/*/Community/MSBuild/Current/Bin"
    )

    if(NOT MSBUILD_EXE)
        message(WARNING "MSBuild.exe not found — skipping GameScripts build.")
    else()
        add_custom_target(BuildGameScripts ALL
            COMMAND ${CMAKE_COMMAND} -E echo "=== Building GameScripts.sln ==="
            COMMAND "${MSBUILD_EXE}" "${GAMESCRIPTS_SLN}" /t:Build /p:Configuration=$<CONFIG>
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/GameScripts"
            COMMENT "Building GameScripts to produce GameScripts.dll"
            VERBATIM
        )
        add_dependencies(${CMAKE_PROJECT_NAME} BuildGameScripts)
    endif()
else()
    message(WARNING "GameScripts.sln not found at ${GAMESCRIPTS_SLN}")
endif()

# Copy GameScripts DLL
set(GAMESCRIPTS_DLL_PATH "${CMAKE_SOURCE_DIR}/GameScripts/bin/$<CONFIG>/GameScripts.dll")
set(GAMESCRIPTS_TARGET_DIR "${PRIMARY_GAME_DIR}/Assets/Scripts")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying GameScripts.dll from: ${GAMESCRIPTS_DLL_PATH}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GAMESCRIPTS_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GAMESCRIPTS_DLL_PATH}"
        "${GAMESCRIPTS_TARGET_DIR}/GameScripts.dll"
    VERBATIM
)


# ============== Organize files (Visual Studio) ============
set(CE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/CoreEngine/src)
set(CE_INC ${CMAKE_CURRENT_SOURCE_DIR}/CoreEngine/include)
set(RG_SRC "${PRIMARY_GAME_DIR}/src")
set(RG_INC "${PRIMARY_GAME_DIR}/include")

source_group("BaseEngine\\Components" FILES
    ${CE_SRC}/Camera.cpp
    ${CE_INC}/Camera.h
    ${CE_SRC}/glapp.cpp
    ${CE_INC}/glapp.h
)

source_group("BaseEngine\\CoreEngine" FILES
    ${CE_SRC}/CoreEngine.cpp
    ${CE_INC}/CoreEngine.h
)

source_group("BaseEngine\\Serialization" FILES
    ${CE_SRC}/FilePath.cpp
    ${CE_INC}/FilePath.h
    ${CE_SRC}/FileIO.cpp
    ${CE_INC}/FileIO.h
    ${CE_INC}/DataDefs.h
    ${CE_SRC}/JsonLoad.cpp
    ${CE_INC}/JsonLoad.h
)

source_group("BaseEngine\\Mono Scripting" FILES
    ${CE_SRC}/MonoManager.cpp
    ${CE_INC}/MonoManager.h
    ${CE_INC}/MonoSystem.cpp
    ${CE_INC}/MonoSystem.h
    ${CE_SRC}/MonoBindings.cpp
)

source_group("BaseEngine\\CScripting" FILES
    ${CE_INC}/BehaviourFCT.h
    ${CE_INC}/BehaviourComponent.h
    ${CE_INC}/LogicSystem.h
    ${CE_SRC}/LogicSystem.cpp
    ${CE_INC}/GameScripts.h
)

source_group("BaseEngine\\Debug" FILES
    ${CE_SRC}/Debug.cpp
    ${CE_INC}/Debug.h
    ${CE_INC}/CrashHandler.h
)

source_group("BaseEngine\\ECS\\Registry" FILES
    ${CE_INC}/Registry.h
    ${CE_SRC}/Registry.cpp
)

source_group("BaseEngine\\ECS\\Entity" FILES
    ${CE_INC}/Entity.h
    ${CE_INC}/EntityManager.h
    ${CE_SRC}/EntityManager.cpp
)

source_group("BaseEngine\\ECS\\Component" FILES
    ${CE_INC}/Component.h
    ${CE_INC}/ComponentManager.h
)

source_group("BaseEngine\\ECS\\Component\\Scripting" FILES
    ${CE_INC}/MonoScriptComponent.h
)

source_group("BaseEngine\\ECS\\Component\\Graphics" FILES
    ${CE_SRC}/Sprite.cpp
    ${CE_INC}/Sprite.h
    ${CE_SRC}/Animator.cpp
    ${CE_INC}/Animator.h
    ${CE_SRC}/RenderComponent.cpp
    ${CE_INC}/RenderComponent.h
)

source_group("BaseEngine\\ECS\\Component\\Physics" FILES
    ${CE_SRC}/Physics.cpp
    ${CE_INC}/Physics.h
    ${CE_SRC}/ForcesManager.cpp
    ${CE_INC}/ForcesManager.h
)

source_group("BaseEngine\\Systems" FILES
    ${CE_INC}/System.h
)

source_group("BaseEngine\\Systems\\Audio" FILES
    ${CE_INC}/AudioSystem.h
)

source_group("BaseEngine\\Systems\\Render" FILES
    ${CE_INC}/RenderSystem.h
    ${CE_SRC}/RenderSystem.cpp
)

source_group("BaseEngine\\Systems\\Editor" FILES
    ${CE_SRC}/ImguiSystem.cpp
    ${CE_INC}/ImguiSystem.h
    ${CE_SRC}/GuiSystem.cpp
    ${CE_INC}/GuiSystem.h
)

source_group("BaseEngine\\Systems\\Physics" FILES
    ${CE_INC}/PhysicsSystem.h
    ${CE_SRC}/PhysicsSystem.cpp
)

source_group("BaseEngine\\Systems\\Undo" FILES
    ${CE_INC}/UndoSystem.h
    ${CE_SRC}/UndoSystem.cpp
)

source_group("BaseEngine\\Managers" FILES
    ${CE_INC}/SceneManager.h
    ${CE_SRC}/SceneManager.cpp
    ${CE_INC}/ReplayManager.h
    ${CE_SRC}/ReplayManager.cpp
)

source_group("BaseEngine\\Managers\\Frame Rate" FILES
    ${CE_SRC}/FrameRateController.cpp
    ${CE_INC}/FrameRateController.h
)

source_group("BaseEngine\\Managers\\Collision" FILES
    ${CE_SRC}/CollisionManager.cpp
    ${CE_INC}/CollisionManager.h
)

source_group("BaseEngine\\Libraries\\Audio" FILES
    ${CE_SRC}/Audio.cpp
    ${CE_SRC}/AudioCache.cpp
    ${CE_INC}/Audio.h
    ${CE_INC}/AudioCache.h
    ${CE_SRC}/AudioComponent.cpp
    ${CE_INC}/AudioComponent.h
)

source_group("BaseEngine\\Libraries\\Input" FILES
    ${CE_SRC}/Input.cpp
    ${CE_INC}/Input.h
)

source_group("BaseEngine\\Libraries\\Math" FILES
    ${CE_SRC}/Remath.cpp
    ${CE_INC}/Remath.h
)

source_group("BaseEngine\\Libraries\\Render" FILES
    ${CE_SRC}/Render.cpp
    ${CE_INC}/Render.h
    ${CE_SRC}/SimpleAtlas.cpp
    ${CE_INC}/SimpleAtlas.h
)

source_group("BaseEngine\\Libraries\\Collision" FILES
    ${CE_SRC}/Collision.cpp
    ${CE_INC}/Collision.h
    ${CE_SRC}/SpatialGrid.cpp
    ${CE_INC}/SpatialGrid.h
)

source_group("others" FILES
    ${CE_SRC}/Movement.cpp
    ${CE_SRC}/MovementSystem.cpp
    ${CE_INC}/Movement.h
    ${CE_INC}/MovementSystem.h
    ${CE_SRC}/TextSystem.cpp
    ${CE_INC}/TextSystem.h
    ${CE_INC}/MemoryPool.h
    ${CE_INC}/GameMemoryManager.h
    ${CE_INC}/PooledEntityFactory.h
)

source_group("others\\EventBus" FILES
    ${CE_SRC}/EventBus.cpp
    ${CE_INC}/EventBus.h
)

# GAME FILES
source_group("Game\\Main" FILES
    ${RG_SRC}/main.cpp
)

source_group("Game\\Combat System" FILES
    ${RG_INC}/CombatSystem.h
    ${RG_SRC}/CombatSystem.cpp
)

source_group("Game\\Game Logic" FILES
    ${RG_INC}/PlayerSystem.h
    ${RG_SRC}/PlayerSystem.cpp
    ${RG_SRC}/Spells.cpp
    ${RG_INC}/Spells.h
    ${RG_SRC}/Fretboard.cpp
    ${RG_INC}/Fretboard.h
    ${RG_SRC}/EntityCreation.cpp
    ${RG_INC}/EntityCreation.h
    ${RG_INC}/EnemySystem.h
    ${RG_SRC}/EnemySystem.cpp
    ${RG_INC}/Enemy.h
)
