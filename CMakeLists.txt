cmake_minimum_required(VERSION 3.16)

# Project definition - C and C++11
project(vglite_render
    VERSION 1.0.0
    LANGUAGES C CXX
    DESCRIPTION "Independent VGLite-compatible rendering library with ThorVG backend"
)

# C++14 standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export all symbols for Windows DLL
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# =============================================================================
# Options
# =============================================================================

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(VGLITE_BUILD_TESTS "Build unit tests" ON)
option(VGLITE_BUILD_EXAMPLES "Build example programs" ON)

# Backend selection
option(VG_LITE_USE_GL_BACKEND "Use ThorVG OpenGL backend (default: OFF, use SW)" OFF)

if(VG_LITE_USE_GL_BACKEND)
    add_compile_definitions(VG_LITE_RENDER_BACKEND=2)  # VG_LITE_RENDER_GL
    message(STATUS "Using ThorVG OpenGL backend")
else()
    add_compile_definitions(VG_LITE_RENDER_BACKEND=1)  # VG_LITE_RENDER_SW
    message(STATUS "Using ThorVG Software backend")
endif()

# =============================================================================
# Output directories
# =============================================================================

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# =============================================================================
# Platform detection
# =============================================================================

if(ANDROID)
    set(VGLITE_PLATFORM "android")
    add_definitions(-DVGLITE_PLATFORM_ANDROID)
elseif(WIN32)
    set(VGLITE_PLATFORM "windows")
    add_definitions(-DVGLITE_PLATFORM_WINDOWS)
elseif(APPLE)
    set(VGLITE_PLATFORM "macos")
    add_definitions(-DVGLITE_PLATFORM_MACOS)
elseif(UNIX)
    set(VGLITE_PLATFORM "linux")
    add_definitions(-DVGLITE_PLATFORM_LINUX)
else()
    set(VGLITE_PLATFORM "unknown")
endif()

# MSVC specific settings
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    if(BUILD_SHARED_LIBS)
        add_compile_definitions(VGLITE_EXPORTS)
    endif()
endif()

# =============================================================================
# ThorVG submodule
# =============================================================================

set(VGLITE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(third_party)

# =============================================================================
# VGLite library
# =============================================================================

# Source files
set(VGLITE_SOURCES
    ${VGLITE_ROOT_DIR}/src/vg_lite_matrix.cpp
    ${VGLITE_ROOT_DIR}/src/vg_lite_tvg.cpp
)

# Create library
add_library(vglite_render ${VGLITE_SOURCES})

add_library(vglite::render ALIAS vglite_render)

target_include_directories(vglite_render PUBLIC
    $<BUILD_INTERFACE:${VGLITE_ROOT_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ThorVG include and link
target_include_directories(vglite_render PRIVATE
    ${VGLITE_ROOT_DIR}/third_party/thorvg/inc
    ${CMAKE_SOURCE_DIR}/tests/stb
)

if(TARGET thorvg)
    target_link_libraries(vglite_render PRIVATE thorvg)
else()
    message(WARNING "ThorVG target not found - build may fail")
endif()

# MSVC DLL export
if(MSVC AND BUILD_SHARED_LIBS)
    target_compile_definitions(vglite_render
        INTERFACE "VGLITE_API=__declspec(dllimport)"
        PRIVATE "VGLITE_API=__declspec(dllexport)"
    )
endif()

# =============================================================================
# Tests
# =============================================================================

if(VGLITE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# =============================================================================
# Examples
# =============================================================================

if(VGLITE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# =============================================================================
# Installation
# =============================================================================

include(GNUInstallDirs)

install(TARGETS vglite_render
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${VGLITE_ROOT_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)
