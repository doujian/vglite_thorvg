/**
 * @file vg_lite_matrix.c
 * @brief Matrix transformation operations
 */

/*********************
 *      INCLUDES
 *********************/

#include <math.h>
#include <string.h>
#include "../include/vg_lite.h"

/*********************
 *      DEFINES
 *********************/

#define VG_SW_BLIT_PRECISION_OPT 1

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

vg_lite_error_t vg_lite_identity(vg_lite_matrix_t * matrix)
{
    /* Set identify matrix. */
    matrix->m[0][0] = 1.0f;
    matrix->m[0][1] = 0.0f;
    matrix->m[0][2] = 0.0f;
    matrix->m[1][0] = 0.0f;
    matrix->m[1][1] = 1.0f;
    matrix->m[1][2] = 0.0f;
    matrix->m[2][0] = 0.0f;
    matrix->m[2][1] = 0.0f;
    matrix->m[2][2] = 1.0f;

#if VG_SW_BLIT_PRECISION_OPT
    matrix->scaleX = 1.0f;
    matrix->scaleY = 1.0f;
    matrix->angle   = 0.0f;
#endif /* VG_SW_BLIT_PRECISION_OPT */

    return VG_LITE_SUCCESS;
}

static void multiply(vg_lite_matrix_t * matrix, vg_lite_matrix_t * mult)
{
    vg_lite_matrix_t temp;
    int row, column;

    /* Process all rows. */
    for(row = 0; row < 3; row++) {
        /* Process all columns. */
        for(column = 0; column < 3; column++) {
            /* Compute matrix entry. */
            temp.m[row][column] = (matrix->m[row][0] * mult->m[0][column])
                                  + (matrix->m[row][1] * mult->m[1][column])
                                  + (matrix->m[row][2] * mult->m[2][column]);
        }
    }

    /* Copy temporary matrix into result. */
    memcpy(matrix, &temp, sizeof(vg_lite_float_t) * 9);
}

vg_lite_error_t vg_lite_translate(vg_lite_float_t x, vg_lite_float_t y, vg_lite_matrix_t * matrix)
{
    vg_lite_matrix_t t;

    if (matrix == NULL) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    /* Set translation matrix. */
    t.m[0][0] = 1.0f; t.m[0][1] = 0.0f; t.m[0][2] = x;
    t.m[1][0] = 0.0f; t.m[1][1] = 1.0f; t.m[1][2] = y;
    t.m[2][0] = 0.0f; t.m[2][1] = 0.0f; t.m[2][2] = 1.0f;
    t.scaleX = 1.0f; t.scaleY = 1.0f; t.angle = 0.0f;

    /* Multiply with current matrix. */
    multiply(matrix, &t);

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_scale(vg_lite_float_t scale_x, vg_lite_float_t scale_y, vg_lite_matrix_t * matrix)
{
    vg_lite_matrix_t s;

    if (matrix == NULL) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    /* Set scale matrix. */
    s.m[0][0] = scale_x; s.m[0][1] = 0.0f;    s.m[0][2] = 0.0f;
    s.m[1][0] = 0.0f;    s.m[1][1] = scale_y; s.m[1][2] = 0.0f;
    s.m[2][0] = 0.0f;    s.m[2][1] = 0.0f;    s.m[2][2] = 1.0f;
    s.scaleX = 1.0f; s.scaleY = 1.0f; s.angle = 0.0f;

    /* Multiply with current matrix. */
    multiply(matrix, &s);

#if VG_SW_BLIT_PRECISION_OPT
    /* Update scale tracking */
    matrix->scaleX = matrix->scaleX * scale_x;
    matrix->scaleY = matrix->scaleY * scale_y;
#endif

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_rotate(vg_lite_float_t degrees, vg_lite_matrix_t * matrix)
{
    vg_lite_matrix_t r;
    vg_lite_float_t angle, cos_angle, sin_angle;

    if (matrix == NULL) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    /* Convert degrees into radians. */
    angle = (degrees / 180.0f) * 3.141592654f;

    /* Compute cosine and sine values. */
    cos_angle = cosf(angle);
    sin_angle = sinf(angle);

    /* Set rotation matrix. */
    r.m[0][0] = cos_angle;  r.m[0][1] = -sin_angle; r.m[0][2] = 0.0f;
    r.m[1][0] = sin_angle;  r.m[1][1] = cos_angle;  r.m[1][2] = 0.0f;
    r.m[2][0] = 0.0f;       r.m[2][1] = 0.0f;       r.m[2][2] = 1.0f;
    r.scaleX = 1.0f; r.scaleY = 1.0f; r.angle = 0.0f;

    /* Multiply with current matrix. */
    multiply(matrix, &r);

#if VG_SW_BLIT_PRECISION_OPT
    /* Update angle tracking */
    matrix->angle = matrix->angle + degrees;
    if(matrix->angle >= 360.0f) {
        vg_lite_uint32_t count = (vg_lite_uint32_t)(matrix->angle / 360.0f);
        matrix->angle = matrix->angle - (count * 360.0f);
    }
#endif

    return VG_LITE_SUCCESS;
}
