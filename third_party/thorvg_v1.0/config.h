#ifndef THORVG_CONFIG_H
#define THORVG_CONFIG_H

#define THORVG_VERSION_STRING "1.0.0-vglite"

/* Thread support */
#define THORVG_THREAD_SUPPORT 1

/* Renderers - enable both SW and GL */
#define THORVG_SW_RASTER_SUPPORT 1
#define THORVG_GL_RASTER_SUPPORT 1
#define THORVG_GL_GLES 1

/* Partial render - not needed */
/* #undef THORVG_PARTIAL_RENDER_SUPPORT */

/* Loaders - only SVG enabled for minimal build */
#define THORVG_SVG_LOADER_SUPPORT 1

/* PNG/JPG/TTF/WebP loaders - NOT defined (not #define to 0) */

/* Savers - not needed */

#endif
