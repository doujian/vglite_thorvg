/**
 * @file vglite_jni.cpp
 * @brief JNI bridge for VGLite Android example with EGL/OpenGL ES
 * 
 * Provides JNI functions for vg_lite initialization with EGL,
 * color cycling animation rendering at ~60 FPS.
 */

#include <jni.h>
#include <string.h>
#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>

// EGL and OpenGL ES includes
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

// Use GL backend for EGL/OpenGL ES rendering
#define VG_LITE_RENDER_BACKEND 2
#include "vg_lite.h"

// ============================================================================
// Global Variables
// ============================================================================

// VGLite render buffer
static vg_lite_buffer_t* g_renderBuffer = nullptr;
static int g_width = 0;
static int g_height = 0;

// EGL globals
static EGLDisplay g_eglDisplay = EGL_NO_DISPLAY;
static EGLContext g_eglContext = EGL_NO_CONTEXT;
static EGLSurface g_eglSurface = EGL_NO_SURFACE;
static ANativeWindow* g_nativeWindow = nullptr;

// Render thread
static std::thread* g_renderThread = nullptr;
static std::atomic<bool> g_rendering(false);

// Color cycling
static const uint32_t g_colors[] = {
    0xFFFF0000,  // Red
    0xFF00FF00,  // Green
    0xFF0000FF,  // Blue
    0xFFFFFF00,  // Yellow
    0xFF00FFFF   // Cyan
};
static int g_colorIndex = 0;
static int g_frameCount = 0;

extern "C" {

// ============================================================================
// Initialization with Surface
// ============================================================================

/**
 * Initialize VGLite with EGL/OpenGL ES using a Surface from Java.
 * 
 * @param surface Android Surface object from Java
 * @return true if successful, false otherwise
 */
JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeInitWithSurface(
    JNIEnv* env, jobject thiz, jobject surface) {
    
    // Get ANativeWindow from Surface
    g_nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (!g_nativeWindow) {
        return JNI_FALSE;
    }
    
    // Get window dimensions
    g_width = ANativeWindow_getWidth(g_nativeWindow);
    g_height = ANativeWindow_getHeight(g_nativeWindow);
    
    // Initialize EGL
    g_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_eglDisplay == EGL_NO_DISPLAY) {
        ANativeWindow_release(g_nativeWindow);
        g_nativeWindow = nullptr;
        return JNI_FALSE;
    }
    
    EGLint major, minor;
    if (!eglInitialize(g_eglDisplay, &major, &minor)) {
        ANativeWindow_release(g_nativeWindow);
        g_nativeWindow = nullptr;
        g_eglDisplay = EGL_NO_DISPLAY;
        return JNI_FALSE;
    }
    
    // Choose config
    EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(g_eglDisplay, attribs, &config, 1, &numConfigs) || numConfigs < 1) {
        eglTerminate(g_eglDisplay);
        ANativeWindow_release(g_nativeWindow);
        g_nativeWindow = nullptr;
        g_eglDisplay = EGL_NO_DISPLAY;
        return JNI_FALSE;
    }
    
    // Create context (OpenGL ES 3.0 for glBlitFramebuffer support)
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    g_eglContext = eglCreateContext(g_eglDisplay, config, NULL, contextAttribs);
    if (g_eglContext == EGL_NO_CONTEXT) {
        eglTerminate(g_eglDisplay);
        ANativeWindow_release(g_nativeWindow);
        g_nativeWindow = nullptr;
        g_eglDisplay = EGL_NO_DISPLAY;
        return JNI_FALSE;
    }
    
    // Create surface
    g_eglSurface = eglCreateWindowSurface(g_eglDisplay, config, g_nativeWindow, NULL);
    if (g_eglSurface == EGL_NO_SURFACE) {
        eglDestroyContext(g_eglDisplay, g_eglContext);
        eglTerminate(g_eglDisplay);
        ANativeWindow_release(g_nativeWindow);
        g_nativeWindow = nullptr;
        g_eglContext = EGL_NO_CONTEXT;
        g_eglDisplay = EGL_NO_DISPLAY;
        return JNI_FALSE;
    }
    
    // Make current
    if (!eglMakeCurrent(g_eglDisplay, g_eglSurface, g_eglSurface, g_eglContext)) {
        eglDestroySurface(g_eglDisplay, g_eglSurface);
        eglDestroyContext(g_eglDisplay, g_eglContext);
        eglTerminate(g_eglDisplay);
        ANativeWindow_release(g_nativeWindow);
        g_nativeWindow = nullptr;
        g_eglSurface = EGL_NO_SURFACE;
        g_eglContext = EGL_NO_CONTEXT;
        g_eglDisplay = EGL_NO_DISPLAY;
        return JNI_FALSE;
    }
    
    // Initialize VGLite with GL backend
    vg_lite_error_t error = vg_lite_init(256, 256);
    if (error != VG_LITE_SUCCESS) {
        eglMakeCurrent(g_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(g_eglDisplay, g_eglSurface);
        eglDestroyContext(g_eglDisplay, g_eglContext);
        eglTerminate(g_eglDisplay);
        ANativeWindow_release(g_nativeWindow);
        g_nativeWindow = nullptr;
        g_eglSurface = EGL_NO_SURFACE;
        g_eglContext = EGL_NO_CONTEXT;
        g_eglDisplay = EGL_NO_DISPLAY;
        return JNI_FALSE;
    }
    
    // Allocate render buffer
    g_renderBuffer = new vg_lite_buffer_t();
    memset(g_renderBuffer, 0, sizeof(vg_lite_buffer_t));
    g_renderBuffer->width = g_width;
    g_renderBuffer->height = g_height;
    g_renderBuffer->format = VG_LITE_BGRA8888;
    
    error = vg_lite_allocate(g_renderBuffer);
    if (error != VG_LITE_SUCCESS) {
        delete g_renderBuffer;
        g_renderBuffer = nullptr;
        vg_lite_close();
        eglMakeCurrent(g_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(g_eglDisplay, g_eglSurface);
        eglDestroyContext(g_eglDisplay, g_eglContext);
        eglTerminate(g_eglDisplay);
        ANativeWindow_release(g_nativeWindow);
        g_nativeWindow = nullptr;
        g_eglSurface = EGL_NO_SURFACE;
        g_eglContext = EGL_NO_CONTEXT;
        g_eglDisplay = EGL_NO_DISPLAY;
        return JNI_FALSE;
    }
    
    // Reset color cycling state
    g_colorIndex = 0;
    g_frameCount = 0;
    
    return JNI_TRUE;
}

// ============================================================================
// Render Thread
// ============================================================================

/**
 * Start the render thread with color cycling animation.
 */
JNIEXPORT void JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeRenderStart(JNIEnv* env, jobject thiz) {
    g_rendering = true;
    g_renderThread = new std::thread([]() {
        while (g_rendering) {
            // Clear with cycling color
            if (g_renderBuffer) {
                vg_lite_clear(g_renderBuffer, nullptr, g_colors[g_colorIndex]);
                vg_lite_finish();
            }
            
            // Swap buffers
            if (g_eglDisplay != EGL_NO_DISPLAY && g_eglSurface != EGL_NO_SURFACE) {
                eglSwapBuffers(g_eglDisplay, g_eglSurface);
            }
            
            // Update color every 60 frames (~1 second at 60 FPS)
            g_frameCount++;
            if (g_frameCount >= 60) {
                g_frameCount = 0;
                g_colorIndex = (g_colorIndex + 1) % 5;
            }
            
            // ~60 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    });
}

/**
 * Stop the render thread.
 */
JNIEXPORT void JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeRenderStop(JNIEnv* env, jobject thiz) {
    g_rendering = false;
    if (g_renderThread) {
        g_renderThread->join();
        delete g_renderThread;
        g_renderThread = nullptr;
    }
}

// ============================================================================
// Cleanup
// ============================================================================

/**
 * Free all resources including EGL and VGLite.
 */
JNIEXPORT void JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeCleanup(JNIEnv* env, jobject thiz) {
    // Stop render thread first
    g_rendering = false;
    if (g_renderThread) {
        g_renderThread->join();
        delete g_renderThread;
        g_renderThread = nullptr;
    }
    
    // Cleanup VGLite render buffer
    if (g_renderBuffer) {
        vg_lite_free(g_renderBuffer);
        delete g_renderBuffer;
        g_renderBuffer = nullptr;
    }
    
    // Close VGLite
    vg_lite_close();
    
    // Cleanup EGL
    if (g_eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_eglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(g_eglDisplay, g_eglSurface);
            g_eglSurface = EGL_NO_SURFACE;
        }
        if (g_eglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(g_eglDisplay, g_eglContext);
            g_eglContext = EGL_NO_CONTEXT;
        }
        eglTerminate(g_eglDisplay);
        g_eglDisplay = EGL_NO_DISPLAY;
    }
    
    // Release native window
    if (g_nativeWindow) {
        ANativeWindow_release(g_nativeWindow);
        g_nativeWindow = nullptr;
    }
    
    // Reset dimensions
    g_width = 0;
    g_height = 0;
    g_colorIndex = 0;
    g_frameCount = 0;
}

// ============================================================================
// Legacy Methods (for backward compatibility)
// ============================================================================

JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeInit(JNIEnv* env, jobject thiz,
                                                   jint width, jint height) {
    // Legacy init without EGL - not used in new implementation
    g_width = width;
    g_height = height;
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeClear(JNIEnv* env, jobject thiz) {
    if (g_renderBuffer == nullptr) {
        return JNI_FALSE;
    }
    vg_lite_error_t error = vg_lite_clear(g_renderBuffer, nullptr, g_colors[g_colorIndex]);
    return (error == VG_LITE_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeRender(JNIEnv* env, jobject thiz) {
    if (g_renderBuffer == nullptr) {
        return JNI_FALSE;
    }
    vg_lite_error_t error = vg_lite_finish();
    return (error == VG_LITE_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeGetBufferMemory(JNIEnv* env, jobject thiz) {
    if (g_renderBuffer == nullptr) {
        return 0;
    }
    return (jlong)g_renderBuffer->memory;
}

JNIEXPORT jint JNICALL
Java_com_example_vglite_VGLiteRenderer_nativeGetBufferStride(JNIEnv* env, jobject thiz) {
    if (g_renderBuffer == nullptr) {
        return 0;
    }
    return g_renderBuffer->stride;
}

} // extern "C"
