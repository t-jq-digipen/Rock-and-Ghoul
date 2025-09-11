# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 2.8...3.13)
cmake_policy(VERSION 3.5)
project(aviddivas)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(ImportDependencies)
importDependencies()

add_definitions(-DGLEW_STATIC)
add_definitions(-DUSE_CSD3151_AUTOMATION=0)

set(ALL_LIBS
  PRIVATE glfw
  PRIVATE libglew_static
  PRIVATE imgui
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/glm)

# ======================= Directories =============================
set(Audio_DIR         ./Build/Audio)
set(Camera_DIR        ./Build/Camera)
set(Collision_DIR     ./Build/Collision)
set(EntityManger_DIR  ./Build/EntityManger)
set(GameManager_DIR   ./Build/GameManager)
set(Graphics_DIR      ./Build/Graphics)
set(LevelEditor_DIR   ./Build/LevelEditor)
set(Logic_DIR         ./Build/Logic)
set(Main_DIR          ./Build/Main)
set(Physics_DIR       ./Build/Physics)
set(Serialization_DIR ./Build/Serialization)
set(Transform_DIR     ./Build/Transform)
set(Input_DIR 	      ./Build/Input)
set(Extern_DIR        ./Build/Extern)

# ======================= File Globbing ============================
file(GLOB_RECURSE Audio_SRC        "${Audio_DIR}/*.cpp")
file(GLOB_RECURSE Audio_HEAD       "${Audio_DIR}/*.h" "${Audio_DIR}/*.hpp")

file(GLOB_RECURSE Camera_SRC       "${Camera_DIR}/*.cpp")
file(GLOB_RECURSE Camera_HEAD      "${Camera_DIR}/*.h" "${Camera_DIR}/*.hpp")

file(GLOB_RECURSE Collision_SRC    "${Collision_DIR}/*.cpp")
file(GLOB_RECURSE Collision_HEAD   "${Collision_DIR}/*.h" "${Collision_DIR}/*.hpp")

file(GLOB_RECURSE EntityManger_SRC "${EntityManger_DIR}/*.cpp")
file(GLOB_RECURSE EntityManger_HEAD "${EntityManger_DIR}/*.h" "${EntityManger_DIR}/*.hpp")

file(GLOB_RECURSE GameManager_SRC  "${GameManager_DIR}/*.cpp")
file(GLOB_RECURSE GameManager_HEAD "${GameManager_DIR}/*.h" "${GameManager_DIR}/*.hpp")

file(GLOB_RECURSE Graphics_SRC     "${Graphics_DIR}/*.cpp")
file(GLOB_RECURSE Graphics_HEAD    "${Graphics_DIR}/*.h" "${Graphics_DIR}/*.hpp")

file(GLOB_RECURSE LevelEditor_SRC  "${LevelEditor_DIR}/*.cpp")
file(GLOB_RECURSE LevelEditor_HEAD "${LevelEditor_DIR}/*.h" "${LevelEditor_DIR}/*.hpp")

file(GLOB_RECURSE Logic_SRC        "${Logic_DIR}/*.cpp")
file(GLOB_RECURSE Logic_HEAD       "${Logic_DIR}/*.h" "${Logic_DIR}/*.hpp")

file(GLOB_RECURSE Main_SRC         "${Main_DIR}/*.cpp")
file(GLOB_RECURSE Main_HEAD        "${Main_DIR}/*.h" "${Main_DIR}/*.hpp")

file(GLOB_RECURSE Physics_SRC      "${Physics_DIR}/*.cpp")
file(GLOB_RECURSE Physics_HEAD     "${Physics_DIR}/*.h" "${Physics_DIR}/*.hpp")

file(GLOB_RECURSE Serialization_SRC "${Serialization_DIR}/*.cpp")
file(GLOB_RECURSE Serialization_HEAD "${Serialization_DIR}/*.h" "${Serialization_DIR}/*.hpp")

file(GLOB_RECURSE Transform_SRC    "${Transform_DIR}/*.cpp")
file(GLOB_RECURSE Transform_HEAD   "${Transform_DIR}/*.h" "${Transform_DIR}/*.hpp")

file(GLOB_RECURSE Transfor_SRC    "${Transform_DIR}/*.cpp")
file(GLOB_RECURSE Transform_HEAD   "${Transform_DIR}/*.h" "${Transform_DIR}/*.hpp")

file(GLOB_RECURSE Transform_SRC    "${Transform_DIR}/*.cpp")
file(GLOB_RECURSE Transform_HEAD   "${Transform_DIR}/*.h" "${Transform_DIR}/*.hpp")

# ======================= Source Groups for Visual Studio ============
set(BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Build)

source_group("Audio\\Header Files" FILES ${Audio_HEAD})
source_group("Audio\\Source Files" FILES ${Audio_SRC})

source_group("Camera\\Header Files" FILES ${Camera_HEAD})
source_group("Camera\\Source Files" FILES ${Camera_SRC})

source_group("Collision\\Header Files" FILES ${Collision_HEAD})
source_group("Collision\\Source Files" FILES ${Collision_SRC})

source_group("EntityManger\\Header Files" FILES ${EntityManger_HEAD})
source_group("EntityManger\\Source Files" FILES ${EntityManger_SRC})

source_group("GameManager\\Header Files" FILES ${GameManager_HEAD})
source_group("GameManager\\Source Files" FILES ${GameManager_SRC})

source_group("Graphics\\Header Files" FILES ${Graphics_HEAD})
source_group("Graphics\\Source Files" FILES ${Graphics_SRC})

source_group("LevelEditor\\Header Files" FILES ${LevelEditor_HEAD})
source_group("LevelEditor\\Source Files" FILES ${LevelEditor_SRC})

source_group("Logic\\Header Files" FILES ${Logic_HEAD})
source_group("Logic\\Source Files" FILES ${Logic_SRC})

source_group("Main\\Header Files" FILES ${Main_HEAD})
source_group("Main\\Source Files" FILES ${Main_SRC})

source_group("Physics\\Header Files" FILES ${Physics_HEAD})
source_group("Physics\\Source Files" FILES ${Physics_SRC})

source_group("Serialization\\Header Files" FILES ${Serialization_HEAD})
source_group("Serialization\\Source Files" FILES ${Serialization_SRC})

source_group("Transform\\Header Files" FILES ${Transform_HEAD})
source_group("Transform\\Source Files" FILES ${Transform_SRC})
# ======================= Collect All Sources =========================
set(ALL_SOURCES
    ${Audio_SRC}
    ${Camera_SRC}
    ${Collision_SRC}
    ${EntityManger_SRC}
    ${GameManager_SRC}
    ${Graphics_SRC}
    ${LevelEditor_SRC}
    ${Logic_SRC}
    ${Main_SRC}
    ${Physics_SRC}
    ${Serialization_SRC}
    ${Transform_SRC}
)

set(ALL_HEADERS
    ${Audio_HEAD}
    ${Camera_HEAD}
    ${Collision_HEAD}
    ${EntityManger_HEAD}
    ${GameManager_HEAD}
    ${Graphics_HEAD}
    ${LevelEditor_HEAD}
    ${Logic_HEAD}
    ${Main_HEAD}
    ${Physics_HEAD}
    ${Serialization_HEAD}
    ${Transform_HEAD}
)

# ======================= Executable =========================
add_executable(${CMAKE_PROJECT_NAME}
    ${ALL_SOURCES}
    ${ALL_HEADERS}
)

# ======================= Link Libraries =============================
target_link_libraries(${CMAKE_PROJECT_NAME}
    ${ALL_LIBS}
)

# ======================= Set C++ Standard ===========================
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)

# ======================= Compiler Warnings ==========================
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall ${DisableWarnings})
elseif (MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W3 /WX-)
endif()

# ======================= Include Directories ========================
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${Audio_DIR}
    ${Camera_DIR}
    ${Collision_DIR}
    ${EntityManger_DIR}
    ${GameManager_DIR}
    ${Graphics_DIR}
    ${LevelEditor_DIR}
    ${Logic_DIR}
    ${Main_DIR}
    ${Physics_DIR}
    ${Serialization_DIR}
    ${Transform_DIR}
)
