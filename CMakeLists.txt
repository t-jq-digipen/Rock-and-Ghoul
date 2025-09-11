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
    PRIVATE glfw
    PRIVATE libglew_static
    PRIVATE imgui
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


# ======================= Executable =========================
add_executable(${CMAKE_PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# ======================= Link Libraries =============================
target_link_libraries(${CMAKE_PROJECT_NAME}
    ${ALL_LIBS}
)

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
# Virtual folders based on filename patterns
# Virtual folders based on filename patterns

source_group("Main" REGULAR_EXPRESSION "(?i).*(WinMain).*")
source_group("Audio" REGULAR_EXPRESSION "(?i).*(audio|sound|music|speaker|volume|wav|mp3).*")

source_group("Engine" REGULAR_EXPRESSION "(?i).*(engine|core|file|transform|glapp).*")

source_group("Export" REGULAR_EXPRESSION "(?i).*(export|save|load|file|import|data).*")

source_group("Framerate" REGULAR_EXPRESSION "(?i).*(framerate|fps|frame|time|delta|clock|timer).*")

source_group("Graphics" REGULAR_EXPRESSION "(?i).*(render|graphics|shader|mesh|texture|draw|vertex|fragment|opengl|buffer).*")

source_group("Input" REGULAR_EXPRESSION "(?i).*(input|keyboard|mouse|controller|key|button|joystick).*")

source_group("Math" REGULAR_EXPRESSION "(?i).*(math|calculate|algorithm|matrix).*")

source_group("Vector" REGULAR_EXPRESSION "(?i).*(vector|vec2|vec3|vec4|point|coordinate|position).*")

source_group("System" REGULAR_EXPRESSION "(?i).*(system|manager|handler|service).*")

source_group("Types" REGULAR_EXPRESSION "(?i).*(type|struct|class|enum|define|typedef).*")

source_group("Util" REGULAR_EXPRESSION "(?i).*(util|utility|helper|tool|common).*")

source_group("Physics" REGULAR_EXPRESSION "(?i).*(physics|collision|rigidbody|force|gravity|mass|velocity).*")

source_group("Gamestate" REGULAR_EXPRESSION "(?i).*(gamestate|state|scene|level|menu|pause).*")

source_group("Camera" REGULAR_EXPRESSION "(?i).*(camera|view|projection|viewport|look).*")

source_group("Movement" REGULAR_EXPRESSION "(?i).*(movement|move|walk|run|jump|speed|motion).*")




# Catch-all for files that don't match any pattern
source_group("Other" REGULAR_EXPRESSION ".*")