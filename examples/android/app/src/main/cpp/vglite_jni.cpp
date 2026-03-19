/**
 * @file vglite_jni.cpp
 * @brief JNI bridge for VGLite Android example
 * 
 * Provides simple JNI functions for vg_lite initialization,
 * clearing to red color, rendering, and cleanup.
 */

#include <jni.h>
#include <string.h>
#include <cstring>

// Use SW backend for simplicity
#define VG_LITE_RENDER_BACKEND 1
#include "vg_lite.h"

// Global render buffer for the example
static vg_lite_buffer_t* g_renderBuffer = nullptr;
static int g_width = 0;
static int g_height = 0;

// Clear color: deep red (0xFF8B0000 in ARGB format)
#define VG_LITE_CLEAR_COLOR 0xFF8B0000

extern "C" {

// ============================================================================
// Initialization
// ============================================================================

/**
 * Initialize VGLite and create a render buffer.
 * 
 * @param width  Screen width from Java
 * @param height Screen height from Java
 * @return true if successful, false otherwise
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeInit(JNIEnv* env, jobject thiz,
                                                    jint width, jint height) {
    // Initialize VGLite with default tessellation buffer size
    vg_lite_error_t error = vg_lite_init(0, 0);
    if (error != VG_LITE_SUCCESS) {
        return JNI_FALSE;
    }
    
    // Store dimensions
    g_width = width;
    g_height = height;
    
    // Allocate render buffer
    g_renderBuffer = new vg_lite_buffer_t();
    memset(g_renderBuffer, 0, sizeof(vg_lite_buffer_t));
    
    g_renderBuffer->width = width;
    g_renderBuffer->height = height;
    g_renderBuffer->format = VG_LITE_BGRA8888;  // Common Android format
    
    error = vg_lite_allocate(g_renderBuffer);
    if (error != VG_LITE_SUCCESS) {
        delete g_renderBuffer;
        g_renderBuffer = nullptr;
        vg_lite_close();
        return JNI_FALSE;
    }
    
    return JNI_TRUE;
}

// ============================================================================
// Clear Operation
// ============================================================================

/**
 * Clear the render buffer to red color.
 * 
 * @return true if successful, false otherwise
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeClear(JNIEnv* env, jobject thiz) {
    if (g_renderBuffer == nullptr) {
        return JNI_FALSE;
    }
    
    // Clear to deep red color
    vg_lite_error_t error = vg_lite_clear(g_renderBuffer, nullptr, VG_LITE_CLEAR_COLOR);
    return (error == VG_LITE_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

// ============================================================================
// Render Operation
// ============================================================================

/**
 * Render the buffer (flush and finish).
 * 
 * @return true if successful, false otherwise
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeRender(JNIEnv* env, jobject thiz) {
    if (g_renderBuffer == nullptr) {
        return JNI_FALSE;
    }
    
    // Flush the buffer to ensure all operations are visible
    vg_lite_error_t error = vg_lite_flush(g_renderBuffer);
    if (error != VG_LITE_SUCCESS) {
        return JNI_FALSE;
    }
    
    // Wait for all operations to complete
    error = vg_lite_finish();
    return (error == VG_LITE_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

/**
 * Get buffer memory pointer for Java to read pixels.
 * 
 * @return pointer to buffer memory as jlong
 */
JNIEXPORT jlong JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeGetBufferMemory(JNIEnv* env, jobject thiz) {
    if (g_renderBuffer == nullptr) {
        return 0;
    }
    return (jlong)g_renderBuffer->memory;
}

/**
 * Get buffer stride (bytes per row).
 * 
 * @return stride in bytes
 */
JNIEXPORT jint JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeGetBufferStride(JNIEnv* env, jobject thiz) {
    if (g_renderBuffer == nullptr) {
        return 0;
    }
    return g_renderBuffer->stride;
}

// ============================================================================
// Cleanup
// ============================================================================

/**
 * Free resources and close VGLite.
 */
JNIEXPORT void JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeCleanup(JNIEnv* env, jobject thiz) {
    if (g_renderBuffer != nullptr) {
        vg_lite_free(g_renderBuffer);
        delete g_renderBuffer;
        g_renderBuffer = nullptr;
    }
    
    vg_lite_close();
    g_width = 0;
    g_height = 0;
}

} // extern "C"