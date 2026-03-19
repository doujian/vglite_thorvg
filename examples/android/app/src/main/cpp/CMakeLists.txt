cmake_minimum_required(VERSION 3.22.1)
project(vglite_jni)

# Use SW backend for simplicity (no GL context required)
set(VG_LITE_RENDER_BACKEND 1)

# ============================================================================
# VGLite Source Files
# ============================================================================
set(VGLITE_SOURCES
    ../../../../src/vg_lite_matrix.cpp
    ../../../../src/vg_lite_tvg.cpp
)

# ============================================================================
# ThorVG Source Files (v1.0 for SW backend)
# ============================================================================
set(THORVG_DIR ../../../../third_party/thorvg_v1.0)
set(THORVG_SOURCES
    # Common
    ${THORVG_DIR}/common/tvgColor.cpp
    ${THORVG_DIR}/common/tvgCompressor.cpp
    ${THORVG_DIR}/common/tvgMath.cpp
    ${THORVG_DIR}/common/tvgStr.cpp
    # Renderer core
    ${THORVG_DIR}/renderer/tvgAccessor.cpp
    ${THORVG_DIR}/renderer/tvgAnimation.cpp
    ${THORVG_DIR}/renderer/tvgCanvas.cpp
    ${THORVG_DIR}/renderer/tvgFill.cpp
    ${THORVG_DIR}/renderer/tvgInitializer.cpp
    ${THORVG_DIR}/renderer/tvgLoader.cpp
    ${THORVG_DIR}/renderer/tvgPaint.cpp
    ${THORVG_DIR}/renderer/tvgPicture.cpp
    ${THORVG_DIR}/renderer/tvgRender.cpp
    ${THORVG_DIR}/renderer/tvgSaver.cpp
    ${THORVG_DIR}/renderer/tvgScene.cpp
    ${THORVG_DIR}/renderer/tvgShape.cpp
    ${THORVG_DIR}/renderer/tvgTaskScheduler.cpp
    ${THORVG_DIR}/renderer/tvgText.cpp
    # Software rasterizer
    ${THORVG_DIR}/renderer/sw_engine/tvgSwFill.cpp
    ${THORVG_DIR}/renderer/sw_engine/tvgSwImage.cpp
    ${THORVG_DIR}/renderer/sw_engine/tvgSwMath.cpp
    ${THORVG_DIR}/renderer/sw_engine/tvgSwMemPool.cpp
    ${THORVG_DIR}/renderer/sw_engine/tvgSwPostEffect.cpp
    ${THORVG_DIR}/renderer/sw_engine/tvgSwRaster.cpp
    ${THORVG_DIR}/renderer/sw_engine/tvgSwRenderer.cpp
    ${THORVG_DIR}/renderer/sw_engine/tvgSwRle.cpp
    ${THORVG_DIR}/renderer/sw_engine/tvgSwShape.cpp
    ${THORVG_DIR}/renderer/sw_engine/tvgSwStroke.cpp
)

# ============================================================================
# JNI Source
# ============================================================================
set(JNI_SOURCES
    vglite_jni.cpp
)

# ============================================================================
# Include Directories
# ============================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include
    ${THORVG_DIR}
    ${THORVG_DIR}/common
    ${THORVG_DIR}/renderer
    ${THORVG_DIR}/renderer/sw_engine
)

# ============================================================================
# Build Shared Library
# ============================================================================
add_library(vglite_jni SHARED
    ${VGLITE_SOURCES}
    ${THORVG_SOURCES}
    ${JNI_SOURCES}
)

# ============================================================================
# Link Libraries
# ============================================================================
target_link_libraries(vglite_jni
    log
    android
)

# ============================================================================
# Compile Definitions
# ============================================================================
target_compile_definitions(vglite_jni PRIVATE
    THORVG_SW_RASTER_SUPPORT=1
    THORVG_SVG_LOADER_SUPPORT=0
    THORVG_LOTTIE_LOADER_SUPPORT=0
    VG_LITE_RENDER_BACKEND=1
)

# ============================================================================
# Compile Options
# ============================================================================
target_compile_options(vglite_jni PRIVATE
    -fvisibility=hidden
    -Wall
    -Werror
)