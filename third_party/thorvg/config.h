/*
 * VGLite Render - ThorVG Configuration
 * Independent configuration without LVGL dependencies
 */

#ifndef TVG_CONFIG_H
#define TVG_CONFIG_H

/* Enable software rasterizer */
#define THORVG_SW_RASTER_SUPPORT 1

/* GL rasterizer disabled - requires full ThorVG v1.0+ (not LVGL's v0.15)
 * To enable: Replace ThorVG with full v1.0+ source and uncomment:
 * #define THORVG_GL_RASTER_SUPPORT 1
 * #define THORVG_GL_GLES 1
 */

/* Disable SVG/Lottie loaders for minimal build - DO NOT define them at all */
/* #undef THORVG_SVG_LOADER_SUPPORT */
/* #undef THORVG_LOTTIE_LOADER_SUPPORT */

/* ThorVG version */
#define THORVG_VERSION_STRING "0.15.3-vglite"

/* Single threaded (no thread support needed) */
/* #undef THORVG_THREAD_SUPPORT */

#endif /*TVG_CONFIG_H*/
