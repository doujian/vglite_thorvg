/**
 * @file hello_path.c
 * @brief Simple VGLite example: Draw a rectangle using path API
 */

#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"

/* Path data for a simple rectangle: move to top-left, line to corners, close */
/* For VG_LITE_S16 format: each coordinate is int16_t (2 bytes) */
static vg_lite_int16_t path_data[] = {
    /* Move to (50, 50) */
    VLC_OP_MOVE, 50, 50,
    /* Line to (250, 50) */
    VLC_OP_LINE, 250, 50,
    /* Line to (250, 200) */
    VLC_OP_LINE, 250, 200,
    /* Line to (50, 200) */
    VLC_OP_LINE, 50, 200,
    /* Close path */
    VLC_OP_CLOSE,
    /* End */
    VLC_OP_END
};

/* Error checking macro */
#define CHECK_ERROR(result, msg) \
    do { \
        if ((result) != VG_LITE_SUCCESS) { \
            printf("ERROR: %s (code: %d)\n", msg, (result)); \
            return (int)(result); \
        } \
    } while (0)

int main(int argc, char *argv[])
{
    vg_lite_error_t error;
    vg_lite_buffer_t render_buffer;
    vg_lite_path_t path;
    vg_lite_matrix_t matrix;
    
    /* Initialize VGLite with tessellation buffer size 256x256 */
    error = vg_lite_init(256, 256);
    CHECK_ERROR(error, "vg_lite_init failed");
    printf("VGLite initialized successfully\n");
    
    /* Set up render target buffer */
    render_buffer.width = 640;
    render_buffer.height = 480;
    render_buffer.format = VG_LITE_RGBA8888;
    render_buffer.tiled = VG_LITE_LINEAR;
    
    /* Allocate buffer memory */
    error = vg_lite_allocate(&render_buffer);
    CHECK_ERROR(error, "vg_lite_allocate failed");
    printf("Render buffer allocated: %dx%d\n", render_buffer.width, render_buffer.height);
    
    /* Clear the buffer to light blue color (ARGB: 0xFF6495B8) */
    error = vg_lite_clear(&render_buffer, NULL, 0xFF6495B8);
    CHECK_ERROR(error, "vg_lite_clear failed");
    
    /* Calculate path data size */
    vg_lite_uint32_t path_data_size = sizeof(path_data);
    
    /* Initialize the path structure */
    error = vg_lite_init_path(&path,
                              VG_LITE_S16,      /* Coordinate format: signed 16-bit */
                              VG_LITE_MEDIUM,   /* Quality level */
                              path_data_size,   /* Path data length */
                              path_data,        /* Path data */
                              50.0f, 50.0f,     /* Bounding box min */
                              250.0f, 200.0f);  /* Bounding box max */
    CHECK_ERROR(error, "vg_lite_init_path failed");
    printf("Path initialized for rectangle\n");
    
    /* Create identity matrix */
    error = vg_lite_identity(&matrix);
    CHECK_ERROR(error, "vg_lite_identity failed");
    
    /* Draw the rectangle with red fill color (ARGB: 0xFFFF0000) */
    error = vg_lite_draw(&render_buffer,
                         &path,
                         VG_LITE_FILL_NON_ZERO,  /* Fill rule */
                         &matrix,                  /* Transformation matrix */
                         VG_LITE_BLEND_NONE,      /* No blending */
                         0xFFFF0000);             /* Red color */
    CHECK_ERROR(error, "vg_lite_draw failed");
    printf("Rectangle drawn successfully\n");
    
    /* Wait for GPU to finish */
    error = vg_lite_finish();
    CHECK_ERROR(error, "vg_lite_finish failed");
    printf("GPU rendering completed\n");
    
    /* Cleanup resources */
    vg_lite_clear_path(&path);
    vg_lite_free(&render_buffer);
    error = vg_lite_close();
    CHECK_ERROR(error, "vg_lite_close failed");
    
    printf("All resources cleaned up successfully\n");
    printf("Example completed successfully!\n");
    
    (void)argc;  /* Unused */
    (void)argv;  /* Unused */
    
    return 0;
}
