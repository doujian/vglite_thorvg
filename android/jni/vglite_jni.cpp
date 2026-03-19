/**
 * @file vglite_jni.cpp
 * @brief JNI wrapper for VGLite API for Android
 */

#include <jni.h>
#include <string.h>
#include <cstring>

// Include VGLite headers
#define VG_LITE_RENDER_BACKEND 1
#include "vg_lite.h"

extern "C" {

// ============================================================================
// VGLite Initialization
// ============================================================================

/**
 * Initialize VGLite library
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLite_nativeInit(JNIEnv *env, jclass clazz) {
    vg_lite_error_t error = vg_lite_init(0, 0);
    return (error == VG_LITE_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

/**
 * Close VGLite library
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLite_nativeClose(JNIEnv *env, jclass clazz) {
    vg_lite_error_t error = vg_lite_close();
    return (error == VG_LITE_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

// ============================================================================
// Buffer Management
// ============================================================================

/**
 * Create a VGLite buffer
 */
JNIEXPORT jlong JNICALL
Java_com_example_vglite_VGLite_nativeCreateBuffer(JNIEnv *env, jclass clazz,
                                                   jint width, jint height, jint format) {
    vg_lite_buffer_t *buffer = new vg_lite_buffer_t();
    memset(buffer, 0, sizeof(vg_lite_buffer_t));
    
    buffer->width = width;
    buffer->height = height;
    buffer->format = (vg_lite_buffer_format_t)format;
    
    vg_lite_error_t error = vg_lite_allocate(buffer);
    if (error != VG_LITE_SUCCESS) {
        delete buffer;
        return 0;
    }
    
    return (jlong)buffer;
}

/**
 * Free a VGLite buffer
 */
JNIEXPORT void JNICALL
Java_com_example_vglite_VGLite_nativeFreeBuffer(JNIEnv *env, jclass clazz,
                                                  jlong bufferPtr) {
    if (bufferPtr == 0) return;
    
    vg_lite_buffer_t *buffer = (vg_lite_buffer_t *)bufferPtr;
    vg_lite_free(buffer);
    delete buffer;
}

/**
 * Get buffer memory pointer for direct access
 */
JNIEXPORT jlong JNICALL
Java_com_example_vglite_VGLite_nativeGetBufferMemory(JNIEnv *env, jclass clazz,
                                                       jlong bufferPtr) {
    if (bufferPtr == 0) return 0;
    vg_lite_buffer_t *buffer = (vg_lite_buffer_t *)bufferPtr;
    return (jlong)buffer->memory;
}

/**
 * Get buffer width
 */
JNIEXPORT jint JNICALL
Java_com_example_vglite_VGLite_nativeGetBufferWidth(JNIEnv *env, jclass clazz,
                                                      jlong bufferPtr) {
    if (bufferPtr == 0) return 0;
    vg_lite_buffer_t *buffer = (vg_lite_buffer_t *)bufferPtr;
    return buffer->width;
}

/**
 * Get buffer height
 */
JNIEXPORT jint JNICALL
Java_com_example_vglite_VGLite_nativeGetBufferHeight(JNIEnv *env, jclass clazz,
                                                       jlong bufferPtr) {
    if (bufferPtr == 0) return 0;
    vg_lite_buffer_t *buffer = (vg_lite_buffer_t *)bufferPtr;
    return buffer->height;
}

/**
 * Get buffer stride
 */
JNIEXPORT jint JNICALL
Java_com_example_vglite_VGLite_nativeGetBufferStride(JNIEnv *env, jclass clazz,
                                                       jlong bufferPtr) {
    if (bufferPtr == 0) return 0;
    vg_lite_buffer_t *buffer = (vg_lite_buffer_t *)bufferPtr;
    return buffer->stride;
}

// ============================================================================
// Drawing Operations
// ============================================================================

/**
 * Clear buffer to a color
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLite_nativeClear(JNIEnv *env, jclass clazz,
                                            jlong bufferPtr, jint color) {
    if (bufferPtr == 0) return JNI_FALSE;
    
    vg_lite_buffer_t *buffer = (vg_lite_buffer_t *)bufferPtr;
    vg_lite_error_t error = vg_lite_clear(buffer, NULL, color);
    return (error == VG_LITE_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

/**
 * Flush buffer to memory
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLite_nativeFlush(JNIEnv *env, jclass clazz,
                                            jlong bufferPtr) {
    (void)bufferPtr;  // Unused parameter
    vg_lite_error_t error = vg_lite_flush();
    return (error == VG_LITE_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

/**
 * Finish all operations
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLite_nativeFinish(JNIEnv *env, jclass clazz) {
    vg_lite_error_t error = vg_lite_finish();
    return (error == VG_LITE_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

// ============================================================================
// Matrix Operations
// ============================================================================

/**
 * Create identity matrix
 */
JNIEXPORT jlong JNICALL
Java_com_example_vglite_VGLite_nativeCreateMatrix(JNIEnv *env, jclass clazz) {
    vg_lite_matrix_t *matrix = new vg_lite_matrix_t();
    vg_lite_identity(matrix);
    return (jlong)matrix;
}

/**
 * Free matrix
 */
JNIEXPORT void JNICALL
Java_com_example_vglite_VGLite_nativeFreeMatrix(JNIEnv *env, jclass clazz,
                                                 jlong matrixPtr) {
    if (matrixPtr == 0) return;
    vg_lite_matrix_t *matrix = (vg_lite_matrix_t *)matrixPtr;
    delete matrix;
}

/**
 * Apply translation to matrix
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLite_nativeMatrixTranslate(JNIEnv *env, jclass clazz,
                                                       jlong matrixPtr,
                                                       jfloat tx, jfloat ty) {
    if (matrixPtr == 0) return JNI_FALSE;
    vg_lite_matrix_t *matrix = (vg_lite_matrix_t *)matrixPtr;
    vg_lite_translate(tx, ty, matrix);
    return JNI_TRUE;
}

/**
 * Apply scale to matrix
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLite_nativeMatrixScale(JNIEnv *env, jclass clazz,
                                                   jlong matrixPtr,
                                                   jfloat sx, jfloat sy) {
    if (matrixPtr == 0) return JNI_FALSE;
    vg_lite_matrix_t *matrix = (vg_lite_matrix_t *)matrixPtr;
    vg_lite_scale(sx, sy, matrix);
    return JNI_TRUE;
}

/**
 * Apply rotation to matrix
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLite_nativeMatrixRotate(JNIEnv *env, jclass clazz,
                                                    jlong matrixPtr,
                                                    jfloat angle) {
    if (matrixPtr == 0) return JNI_FALSE;
    vg_lite_matrix_t *matrix = (vg_lite_matrix_t *)matrixPtr;
    vg_lite_rotate(angle, matrix);
    return JNI_TRUE;
}

} // extern "C"
