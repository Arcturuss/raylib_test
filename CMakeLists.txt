cmake_minimum_required(VERSION 3.22)
project(raylib-test LANGUAGES C CXX)

# Enable Global Link-Time Optimization
cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported)
    # This forces the linker to aggressively delete unused functions from all libraries
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# ------------------------------------------------------------------------------
# 1. Raylib Dependency (FetchContent)
# ------------------------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
  raylib
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
#   GIT_REPOSITORY https://github.com/raysan5/raylib.git
#   GIT_TAG        master
)
FetchContent_GetProperties(raylib)

set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)

# Raylib options to keep the build light
set(USE_AUDIO OFF CACHE BOOL "" FORCE)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

# ------------------------------------------------------------------------------
# 2. Project Setup
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    main.cpp
)

# ------------------------------------------------------------------------------
# 3. Includes & Linking
# ------------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${raylib_SOURCE_DIR}/src
)

# Link to raylib (this handles OpenGL and system dependencies for you)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    raylib
    # shcore
)

# ------------------------------------------------------------------------------
# 4. Compiler & Linker Options
# ------------------------------------------------------------------------------
if (WIN32)
    # Windows/MinGW specific linking
    target_link_libraries(${PROJECT_NAME} PRIVATE shcore)
    
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0603)
    
    target_link_options(${PROJECT_NAME} PRIVATE 
        -mwindows
        -static
        -static-libgcc
        -static-libstdc++
        -fuse-ld=lld
        -Wl,-pdb=test_osp.pdb
        $<$<CONFIG:Debug>:-g -gcodeview>
        $<$<CONFIG:Release>:-Wl,--gc-sections -s -flto -Wl,-Map=test_osp.map>
    )
elseif (APPLE)
    # macOS specific linking for dead code stripping
    target_link_options(${PROJECT_NAME} PRIVATE 
        $<$<CONFIG:Release>:-Wl,-dead_strip>
    )
endif()

target_compile_options(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-Oz -flto -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti>
)