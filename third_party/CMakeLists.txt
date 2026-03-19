# Third party libraries - ThorVG with optional GL backend support

# Check if GL backend is requested
option(VG_LITE_USE_GL_BACKEND "Use GL backend instead of SW" OFF)

if(VG_LITE_USE_GL_BACKEND)
    message(STATUS "Configuring for GL backend with ThorVG v1.0")
    
    # Use ThorVG v1.0 for GL backend support
    set(THORVG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thorvg_v1.0)
    
    # Common sources
    set(THORVG_COMMON_SOURCES
        ${THORVG_DIR}/common/tvgColor.cpp
        ${THORVG_DIR}/common/tvgCompressor.cpp
        ${THORVG_DIR}/common/tvgMath.cpp
        ${THORVG_DIR}/common/tvgStr.cpp
    )
    
    # Renderer core sources
    set(THORVG_RENDERER_SOURCES
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
    )
    
    # SW engine sources (always include for fallback)
    set(THORVG_SW_SOURCES
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
    
    # GL engine sources
    set(THORVG_GL_SOURCES
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
    )
    
    # SVG loader sources
    set(THORVG_SVG_SOURCES
        ${THORVG_DIR}/loaders/svg/tvgSvgCssStyle.cpp
        ${THORVG_DIR}/loaders/svg/tvgSvgLoader.cpp
        ${THORVG_DIR}/loaders/svg/tvgSvgPath.cpp
        ${THORVG_DIR}/loaders/svg/tvgSvgSceneBuilder.cpp
        ${THORVG_DIR}/loaders/svg/tvgSvgUtil.cpp
        ${THORVG_DIR}/loaders/svg/tvgXmlParser.cpp
    )
    
    # Raw loader (required by tvgLoader.cpp)
    set(THORVG_RAW_SOURCES
        ${THORVG_DIR}/loaders/raw/tvgRawLoader.cpp
    )
    
    # Combine all sources
    set(THORVG_SOURCES
        ${THORVG_COMMON_SOURCES}
        ${THORVG_RENDERER_SOURCES}
        ${THORVG_SW_SOURCES}
        ${THORVG_GL_SOURCES}
        ${THORVG_SVG_SOURCES}
        ${THORVG_RAW_SOURCES}
    )
    
    # Create ThorVG static library
    add_library(thorvg STATIC ${THORVG_SOURCES})
    
    target_include_directories(thorvg PUBLIC
        ${THORVG_DIR}
        ${THORVG_DIR}/common
        ${THORVG_DIR}/renderer
        ${THORVG_DIR}/renderer/sw_engine
        ${THORVG_DIR}/renderer/gl_engine
        ${THORVG_DIR}/loaders/svg
        ${THORVG_DIR}/loaders/raw
    )
    
    # Compile definitions for GL
    target_compile_definitions(thorvg
        PUBLIC TVG_STATIC
        PRIVATE
            THORVG_SW_RASTER_SUPPORT=1
            THORVG_GL_RASTER_SUPPORT=1
            THORVG_GL_TARGET_GL=1    # Use desktop OpenGL (WGL on Windows), not GLES
            THORVG_SVG_LOADER_SUPPORT=1
            THORVG_THREAD_SUPPORT=1
    )
    
    # Find OpenGL
    find_package(OpenGL QUIET)
    if(OpenGL_FOUND)
        target_link_libraries(thorvg PUBLIC OpenGL::GL)
    endif()
    
    # Try to find EGL
    find_library(EGL_LIBRARY EGL QUIET)
    if(EGL_LIBRARY)
        target_link_libraries(thorvg PUBLIC ${EGL_LIBRARY})
    endif()
    
else()
    message(STATUS "Configuring for SW backend with ThorVG v1.0")
    
    # Use ThorVG v1.0 for SW backend
    set(THORVG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thorvg)
    set(THORVG_SRC_DIR ${THORVG_DIR}/src)
    
    # Common sources (src/common/)
    set(THORVG_COMMON_SOURCES
        ${THORVG_SRC_DIR}/common/tvgColor.cpp
        ${THORVG_SRC_DIR}/common/tvgCompressor.cpp
        ${THORVG_SRC_DIR}/common/tvgMath.cpp
        ${THORVG_SRC_DIR}/common/tvgStr.cpp
    )
    
    # Renderer core sources (src/renderer/)
    set(THORVG_RENDERER_SOURCES
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
    )
    
    # SW engine sources (src/renderer/sw_engine/)
    set(THORVG_SW_SOURCES
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
    )
    
    # SVG loader sources (src/loaders/svg/)
    set(THORVG_SVG_SOURCES
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgCssStyle.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgLoader.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgPath.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgSceneBuilder.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgSvgUtil.cpp
        ${THORVG_SRC_DIR}/loaders/svg/tvgXmlParser.cpp
    )
    
    # Raw loader (src/loaders/raw/)
    set(THORVG_RAW_SOURCES
        ${THORVG_SRC_DIR}/loaders/raw/tvgRawLoader.cpp
    )
    
    # Combine all sources
    set(THORVG_SOURCES
        ${THORVG_COMMON_SOURCES}
        ${THORVG_RENDERER_SOURCES}
        ${THORVG_SW_SOURCES}
        ${THORVG_RAW_SOURCES}
    )
    
    # Create ThorVG static library
    add_library(thorvg STATIC ${THORVG_SOURCES})
    
    # Include directories for v1.0
    target_include_directories(thorvg PUBLIC
        ${THORVG_DIR}
        ${THORVG_DIR}/inc
        ${THORVG_SRC_DIR}/common
        ${THORVG_SRC_DIR}/renderer
        ${THORVG_SRC_DIR}/renderer/sw_engine
        ${THORVG_SRC_DIR}/loaders/svg
        ${THORVG_SRC_DIR}/loaders/raw
    )
    
    # Compile definitions for SW only
    target_compile_definitions(thorvg
        PUBLIC TVG_STATIC
        PRIVATE THORVG_SW_RASTER_SUPPORT=1
    )
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(thorvg PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
endif()

if(APPLE)
    target_link_libraries(thorvg PUBLIC "-framework OpenGL")
endif()
