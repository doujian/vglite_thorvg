cmake_minimum_required(VERSION 3.22.1)
project(vglite_android)

# C++14 standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use ThorVG v1.0 with GL backend support
# VG_LITE_USE_GL_BACKEND: Set to ON for GL rendering, OFF for SW rendering
option(VG_LITE_USE_GL_BACKEND "Use GL backend instead of SW" OFF)

# Include VGLite source files
set(VGLITE_SOURCES
    ../../../../../src/vg_lite_matrix.cpp
    ../../../../../src/vg_lite_tvg.cpp
)

# Determine THORVG source directory
if(VG_LITE_USE_GL_BACKEND)
    set(THORVG_DIR ../../../../../third_party/thorvg_v1.0)
    
    # Common sources
    set(THORVG_SOURCES
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
        # GL engine
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
        # Loaders for v1.0
        ${THORVG_DIR}/loaders/raw/tvgRawLoader.cpp
        ${THORVG_DIR}/loaders/svg/tvgSvgLoader.cpp
        ${THORVG_DIR}/loaders/svg/tvgSvgCssStyle.cpp
        ${THORVG_DIR}/loaders/svg/tvgSvgPath.cpp
        ${THORVG_DIR}/loaders/svg/tvgSvgSceneBuilder.cpp
        ${THORVG_DIR}/loaders/svg/tvgSvgUtil.cpp
        ${THORVG_DIR}/loaders/svg/tvgXmlParser.cpp
        ${THORVG_DIR}/loaders/lottie/tvgLottieLoader.cpp
        ${THORVG_DIR}/loaders/lottie/tvgLottieAnimation.cpp
        ${THORVG_DIR}/loaders/lottie/tvgLottieBuilder.cpp
        ${THORVG_DIR}/loaders/lottie/tvgLottieExpressions.cpp
        ${THORVG_DIR}/loaders/lottie/tvgLottieInterpolator.cpp
        ${THORVG_DIR}/loaders/lottie/tvgLottieModel.cpp
        ${THORVG_DIR}/loaders/lottie/tvgLottieModifier.cpp
        ${THORVG_DIR}/loaders/lottie/tvgLottieParser.cpp
        ${THORVG_DIR}/loaders/lottie/tvgLottieParserHandler.cpp
    )
    
    # Include directories
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../include
        ${THORVG_DIR}
        ${THORVG_DIR}/common
        ${THORVG_DIR}/renderer
        ${THORVG_DIR}/renderer/sw_engine
        ${THORVG_DIR}/renderer/gl_engine
        ${THORVG_DIR}/loaders/raw
        ${THORVG_DIR}/loaders/svg
        ${THORVG_DIR}/loaders/lottie
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../tests/stb
    )
    
    # Compile definitions for GL
    set(VGLITE_COMPILE_DEFS
        THORVG_SW_RASTER_SUPPORT=1
        THORVG_GL_RASTER_SUPPORT=1
        THORVG_GL_GLES=1
        THORVG_SVG_LOADER_SUPPORT=1
        THORVG_LOTTIE_LOADER_SUPPORT=1
    )
    
    # Link libraries
    set(VGLITE_LINK_LIBS log android)
    
else()
    # Use SW backend with ThorVG v1.0
    set(THORVG_DIR ../../../../../third_party/thorvg)
    set(THORVG_SRC_DIR ${THORVG_DIR}/src)
    
    # Common sources
    set(THORVG_SOURCES
        ${THORVG_SRC_DIR}/common/tvgColor.cpp
        ${THORVG_SRC_DIR}/common/tvgCompressor.cpp
        ${THORVG_SRC_DIR}/common/tvgMath.cpp
        ${THORVG_SRC_DIR}/common/tvgStr.cpp
        # Renderer core
        ${THORVG_SRC_DIR}/renderer/tvgAccessor.cpp
        ${THORVG_SRC_DIR}/renderer/tvgAnimation.cpp
        ${THORVG_SRC_DIR}/renderer/tvgCanvas.cpp
        ${THORVG_SRC_DIR}/renderer/tvgFill.cpp
        ${THORVG_SRC_DIR}/renderer/tvgInitializer.cpp
        ${THORVG_SRC_DIR}/renderer/tvgLoader.cpp
        ${THORVG_SRC_DIR}/renderer/tvgPaint.cpp
        ${THORVG_SRC_DIR}/renderer/tvgPicture.cpp
        ${THORVG_SRC_DIR}/renderer/tvgRender.cpp
        ${THORVG_SRC_DIR}/renderer/tvgSaver.cpp
        ${THORVG_SRC_DIR}/renderer/tvgScene.cpp
        ${THORVG_SRC_DIR}/renderer/tvgShape.cpp
        ${THORVG_SRC_DIR}/renderer/tvgTaskScheduler.cpp
        ${THORVG_SRC_DIR}/renderer/tvgText.cpp
        # Software rasterizer
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwFill.cpp
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwImage.cpp
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwMath.cpp
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwMemPool.cpp
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwPostEffect.cpp
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwRaster.cpp
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwRenderer.cpp
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwRle.cpp
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwShape.cpp
        ${THORVG_SRC_DIR}/renderer/sw_engine/tvgSwStroke.cpp
        # Loaders
        ${THORVG_SRC_DIR}/loaders/raw/tvgRawLoader.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgLoader.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgCssStyle.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgPath.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgSceneBuilder.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgUtil.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgXmlParser.cpp
        ${THORVG_SRC_DIR}/loaders/lottie/tvgLottieLoader.cpp
        ${THORVG_SRC_DIR}/loaders/lottie/tvgLottieAnimation.cpp
        ${THORVG_SRC_DIR}/loaders/lottie/tvgLottieBuilder.cpp
        ${THORVG_SRC_DIR}/loaders/lottie/tvgLottieExpressions.cpp
        ${THORVG_SRC_DIR}/loaders/lottie/tvgLottieInterpolator.cpp
        ${THORVG_SRC_DIR}/loaders/lottie/tvgLottieModel.cpp
        ${THORVG_SRC_DIR}/loaders/lottie/tvgLottieModifier.cpp
        ${THORVG_SRC_DIR}/loaders/lottie/tvgLottieParser.cpp
        ${THORVG_SRC_DIR}/loaders/lottie/tvgLottieParserHandler.cpp
    )
    
    # Include directories
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../include
        ${THORVG_DIR}
        ${THORVG_DIR}/inc
        ${THORVG_SRC_DIR}/common
        ${THORVG_SRC_DIR}/renderer
        ${THORVG_SRC_DIR}/renderer/sw_engine
        ${THORVG_SRC_DIR}/loaders/raw
        ${THORVG_SRC_DIR}/loaders/svg
        ${THORVG_SRC_DIR}/loaders/lottie
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../tests/stb
    )
    
    # Compile definitions for SW only
    set(VGLITE_COMPILE_DEFS THORVG_SW_RASTER_SUPPORT=1)
    
    # Link libraries
    set(VGLITE_LINK_LIBS log android)
endif()

# Build libvglite_android.so as shared library
add_library(vglite_android SHARED ${VGLITE_SOURCES} ${THORVG_SOURCES} ../../../../jni/vglite_jni.cpp ../../../../../tests/stb_impl.cpp)

# Link libraries
target_link_libraries(vglite_android ${VGLITE_LINK_LIBS})

# Compile definitions
if(VG_LITE_USE_GL_BACKEND)
    target_compile_definitions(vglite_android PRIVATE
        ${VGLITE_COMPILE_DEFS}
        THORVG_SVG_LOADER_SUPPORT=0
        THORVG_LOTTIE_LOADER_SUPPORT=0
        VG_LITE_RENDER_BACKEND=2
    )
else()
    target_compile_definitions(vglite_android PRIVATE
        ${VGLITE_COMPILE_DEFS}
        THORVG_SVG_LOADER_SUPPORT=1
        THORVG_LOTTIE_LOADER_SUPPORT=1
        VG_LITE_RENDER_BACKEND=1
    )
endif()

