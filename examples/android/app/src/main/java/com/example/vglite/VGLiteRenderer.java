package com.example.vglite;

import android.view.Surface;

/**
 * VGLiteRenderer - JNI bridge for VGLite rendering operations with EGL/OpenGL ES.
 * 
 * This class provides an API for initializing VGLite with EGL,
 * starting/stopping color cycling animation, and managing the render lifecycle.
 */
public class VGLiteRenderer {
    
    static {
        // Load the native library
        System.loadLibrary("vglite_jni");
    }
    
    // Screen dimensions
    private int width;
    private int height;
    
    /**
     * Constructor - initializes the renderer with screen dimensions.
     * 
     * @param width  Screen width in pixels
     * @param height Screen height in pixels
     */
    public VGLiteRenderer(int width, int height) {
        this.width = width;
        this.height = height;
    }
    
    /**
     * Initialize VGLite with EGL using a Surface.
     * Must be called before starting rendering.
     * 
     * @param surface Android Surface to render to
     * @return true if initialization succeeded, false otherwise
     */
    public boolean initWithSurface(Surface surface) {
        return nativeInitWithSurface(surface);
    }
    
    /**
     * Start the color cycling animation render thread.
     * Colors cycle: Red -> Green -> Blue -> Yellow -> Cyan
     * Each color displays for approximately 1 second (~60 frames).
     */
    public void renderStart() {
        nativeRenderStart();
    }
    
    /**
     * Stop the render thread.
     * Must be called before cleanup or when pausing.
     */
    public void renderStop() {
        nativeRenderStop();
    }
    
    /**
     * Get screen width.
     * 
     * @return width in pixels
     */
    public int getWidth() {
        return width;
    }
    
    /**
     * Get screen height.
     * 
     * @return height in pixels
     */
    public int getHeight() {
        return height;
    }
    
    /**
     * Release all resources including EGL and VGLite.
     * Must be called when done with the renderer.
     */
    public void cleanup() {
        nativeCleanup();
    }
    
    // ========================================================================
    // Native Methods
    // ========================================================================
    
    /**
     * Initialize VGLite with EGL using a Surface.
     */
    private native boolean nativeInitWithSurface(Object surface);
    
    /**
     * Start the render thread with color cycling.
     */
    private native void nativeRenderStart();
    
    /**
     * Stop the render thread.
     */
    private native void nativeRenderStop();
    
    /**
     * Free all resources and close VGLite.
     */
    private native void nativeCleanup();
    
    // Legacy methods for backward compatibility
    private native boolean nativeInit(int width, int height);
    private native boolean nativeClear();
    private native boolean nativeRender();
    private native long nativeGetBufferMemory();
    private native int nativeGetBufferStride();
}
