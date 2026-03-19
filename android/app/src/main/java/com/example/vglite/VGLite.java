package com.example.vglite;

/**
 * VGLite JNI interface
 * Provides access to VGLite native rendering functions
 */
public class VGLite {
    
    // Load native library
    static {
        System.loadLibrary("vglite_android");
    }
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    /**
     * Initialize VGLite library
     * @return true if successful
     */
    public static native boolean nativeInit();
    
    /**
     * Close VGLite library
     * @return true if successful
     */
    public static native boolean nativeClose();
    
    // ========================================================================
    // Buffer Management
    // ========================================================================
    
    /**
     * Create a VGLite buffer
     * @param width Buffer width in pixels
     * @param height Buffer height in pixels
     * @param format Buffer format (0=BGRA8888)
     * @return Native buffer pointer
     */
    public static native long nativeCreateBuffer(int width, int height, int format);
    
    /**
     * Free a VGLite buffer
     * @param bufferPtr Native buffer pointer
     */
    public static native void nativeFreeBuffer(long bufferPtr);
    
    /**
     * Get buffer memory pointer for direct access
     * @param bufferPtr Native buffer pointer
     * @return Memory pointer
     */
    public static native long nativeGetBufferMemory(long bufferPtr);
    
    /**
     * Get buffer width
     * @param bufferPtr Native buffer pointer
     * @return Buffer width
     */
    public static native int nativeGetBufferWidth(long bufferPtr);
    
    /**
     * Get buffer height
     * @param bufferPtr Native buffer pointer
     * @return Buffer height
     */
    public static native int nativeGetBufferHeight(long bufferPtr);
    
    /**
     * Get buffer stride
     * @param bufferPtr Native buffer pointer
     * @return Buffer stride
     */
    public static native int nativeGetBufferStride(long bufferPtr);
    
    // ========================================================================
    // Drawing Operations
    // ========================================================================
    
    /**
     * Clear buffer to a color
     * @param bufferPtr Native buffer pointer
     * @param color ARGB color value
     * @return true if successful
     */
    public static native boolean nativeClear(long bufferPtr, int color);
    
    /**
     * Flush buffer to memory
     * @param bufferPtr Native buffer pointer
     * @return true if successful
     */
    public static native boolean nativeFlush(long bufferPtr);
    
    /**
     * Finish all operations
     * @return true if successful
     */
    public static native boolean nativeFinish();
    
    // ========================================================================
    // Matrix Operations
    // ========================================================================
    
    /**
     * Create an identity matrix
     * @return Native matrix pointer
     */
    public static native long nativeCreateMatrix();
    
    /**
     * Free a matrix
     * @param matrixPtr Native matrix pointer
     */
    public static native void nativeFreeMatrix(long matrixPtr);
    
    /**
     * Apply translation to matrix
     * @param matrixPtr Native matrix pointer
     * @param tx X translation
     * @param ty Y translation
     * @return true if successful
     */
    public static native boolean nativeMatrixTranslate(long matrixPtr, float tx, float ty);
    
    /**
     * Apply scale to matrix
     * @param matrixPtr Native matrix pointer
     * @param sx X scale factor
     * @param sy Y scale factor
     * @return true if successful
     */
    public static native boolean nativeMatrixScale(long matrixPtr, float sx, float sy);
    
    /**
     * Apply rotation to matrix
     * @param matrixPtr Native matrix pointer
     * @param angle Rotation angle in degrees
     * @return true if successful
     */
    public static native boolean nativeMatrixRotate(long matrixPtr, float angle);
}
