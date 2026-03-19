package com.example.vglite;

/**
 * VGLiteRenderer - JNI bridge for VGLite rendering operations.
 * 
 * This class provides a simple API for initializing VGLite,
 * clearing the buffer to red, and managing the render lifecycle.
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
     * Initialize VGLite and allocate the render buffer.
     * Must be called before any rendering operations.
     * 
     * @return true if initialization succeeded, false otherwise
     */
    public boolean init() {
        return nativeInit(width, height);
    }
    
    /**
     * Clear the render buffer to red color.
     * 
     * @return true if clear succeeded, false otherwise
     */
    public boolean clear() {
        return nativeClear();
    }
    
    /**
     * Render the buffer (flush and finish).
     * 
     * @return true if render succeeded, false otherwise
     */
    public boolean render() {
        return nativeRender();
    }
    
    /**
     * Get the buffer memory pointer.
     * Useful for reading pixel data.
     * 
     * @return pointer to buffer memory as long
     */
    public long getBufferMemory() {
        return nativeGetBufferMemory();
    }
    
    /**
     * Get the buffer stride (bytes per row).
     * 
     * @return stride in bytes
     */
    public int getBufferStride() {
        return nativeGetBufferStride();
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
     * Release all resources and close VGLite.
     * Must be called when done with the renderer.
     */
    public void cleanup() {
        nativeCleanup();
    }
    
    // ========================================================================
    // Native Methods
    // ========================================================================
    
    /**
     * Initialize VGLite and create render buffer.
     */
    private native boolean nativeInit(int width, int height);
    
    /**
     * Clear the buffer to red color.
     */
    private native boolean nativeClear();
    
    /**
     * Render the buffer (flush and finish).
     */
    private native boolean nativeRender();
    
    /**
     * Get buffer memory pointer.
     */
    private native long nativeGetBufferMemory();
    
    /**
     * Get buffer stride.
     */
    private native int nativeGetBufferStride();
    
    /**
     * Free resources and close VGLite.
     */
    private native void nativeCleanup();
}