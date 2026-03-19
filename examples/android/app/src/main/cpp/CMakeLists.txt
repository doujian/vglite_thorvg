cmake_minimum_required(VERSION 3.22.1)
project(vglite_jni)

# Use GL backend for EGL/OpenGL ES rendering
set(VG_LITE_RENDER_BACKEND 2)

# ============================================================================
# VGLite Source Files
# ============================================================================
set(VGLITE_SOURCES
    ../../../../../../src/vg_lite_matrix.cpp
    ../../../../../../src/vg_lite_tvg.cpp
    # STB implementation for PNG export (stbi_write_png)
    ../../../../../../examples/stb_impl.cpp
)

# ============================================================================
# ThorVG Source Files (v1.0 for GL backend with GLES3)
# ============================================================================
set(THORVG_DIR ../../../../../../third_party/thorvg_v1.0)
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
    # Software rasterizer (fallback)
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
    # GL engine (for GLES3 rendering)
    ${THORVG_DIR}/renderer/gl_engine/tvgGl.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlEffect.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlGeometry.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlGpuBuffer.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlProgram.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlRenderPass.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlRenderTarget.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlRenderTask.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlRenderer.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlShader.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlShaderSrc.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlSolidBatch.cpp
    ${THORVG_DIR}/renderer/gl_engine/tvgGlTessellator.cpp
    # SVG loader (required by config.h)
    ${THORVG_DIR}/loaders/svg/tvgSvgLoader.cpp
    ${THORVG_DIR}/loaders/svg/tvgSvgSceneBuilder.cpp
    ${THORVG_DIR}/loaders/svg/tvgSvgPath.cpp
    ${THORVG_DIR}/loaders/svg/tvgSvgUtil.cpp
    ${THORVG_DIR}/loaders/svg/tvgSvgCssStyle.cpp
    ${THORVG_DIR}/loaders/svg/tvgXmlParser.cpp
    # Raw loader (always required)
    ${THORVG_DIR}/loaders/raw/tvgRawLoader.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../tests/stb
    ${THORVG_DIR}
    ${THORVG_DIR}/common
    ${THORVG_DIR}/renderer
    ${THORVG_DIR}/renderer/sw_engine
    ${THORVG_DIR}/renderer/gl_engine
    ${THORVG_DIR}/loaders/svg
    ${THORVG_DIR}/loaders/raw
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
    EGL
    GLESv3
)

# ============================================================================
# Compile Definitions
# NOTE: Do not define THORVG_SVG_LOADER_SUPPORT or THORVG_LOTTIE_LOADER_SUPPORT
# here - they are defined in config.h. Defining them here causes macro
# redefinition errors with -Werror.
# ============================================================================
target_compile_definitions(vglite_jni PRIVATE
    TVG_STATIC
    THORVG_SW_RASTER_SUPPORT=1
    THORVG_GL_RASTER_SUPPORT=1
    THORVG_GL_TARGET_GLES=1
    VG_LITE_RENDER_BACKEND=2
)

# ============================================================================
# Compile Options
# ============================================================================
target_compile_options(vglite_jni PRIVATE
    -fvisibility=hidden
    -Wall
    -Werror
)