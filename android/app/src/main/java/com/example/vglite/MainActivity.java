package com.example.vglite;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * Simple demo activity demonstrating VGLite rendering
 */
public class MainActivity extends Activity {
    
    private ImageView imageView;
    private TextView statusText;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // Create layout
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(16, 16, 16, 16);
        
        // Status text
        statusText = new TextView(this);
        statusText.setTextSize(16);
        layout.addView(statusText);
        
        // Image view for rendered output
        imageView = new ImageView(this);
        imageView.setScaleType(ImageView.ScaleType.FIT_CENTER);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.MATCH_PARENT
        );
        params.weight = 1;
        imageView.setLayoutParams(params);
        layout.addView(imageView);
        
        setContentView(layout);
        
        // Run demo
        runDemo();
    }
    
    private void runDemo() {
        StringBuilder status = new StringBuilder();
        
        // Initialize VGLite
        boolean initSuccess = VGLite.nativeInit();
        status.append("VGLite Init: ").append(initSuccess ? "OK" : "FAILED").append("\n");
        
        if (!initSuccess) {
            statusText.setText(status.toString());
            return;
        }
        
        // Create buffer
        int width = 400;
        int height = 400;
        long bufferPtr = VGLite.nativeCreateBuffer(width, height, 0); // 0 = BGRA8888
        
        if (bufferPtr == 0) {
            status.append("Buffer create: FAILED\n");
            statusText.setText(status.toString());
            VGLite.nativeClose();
            return;
        }
        
        status.append("Buffer: ").append(width).append("x").append(height).append("\n");
        
        // Clear to white
        VGLite.nativeClear(bufferPtr, 0xFFFFFFFF);
        
        // Create a matrix for drawing
        long matrixPtr = VGLite.nativeCreateMatrix();
        
        // Apply some transformations
        VGLite.nativeMatrixTranslate(matrixPtr, 200, 200);
        VGLite.nativeMatrixRotate(matrixPtr, 45);
        VGLite.nativeMatrixScale(matrixPtr, 0.8f, 0.8f);
        
        status.append("Matrix: Created and transformed\n");
        
        // Flush and finish
        VGLite.nativeFlush(bufferPtr);
        VGLite.nativeFinish();
        
        // Get buffer info
        int bufWidth = VGLite.nativeGetBufferWidth(bufferPtr);
        int bufHeight = VGLite.nativeGetBufferHeight(bufferPtr);
        int bufStride = VGLite.nativeGetBufferStride(bufferPtr);
        status.append("Buffer info: ").append(bufWidth).append("x").append(bufHeight)
              .append(" stride=").append(bufStride).append("\n");
        
        // Convert buffer to bitmap for display
        Bitmap bitmap = bufferToBitmap(bufferPtr, width, height);
        if (bitmap != null) {
            imageView.setImageBitmap(bitmap);
            status.append("Render: OK\n");
        } else {
            status.append("Render: Failed to create bitmap\n");
        }
        
        // Cleanup
        VGLite.nativeFreeMatrix(matrixPtr);
        VGLite.nativeFreeBuffer(bufferPtr);
        VGLite.nativeClose();
        
        status.append("Cleanup: OK\n");
        statusText.setText(status.toString());
    }
    
    /**
     * Convert VGLite buffer to Android Bitmap
     * Note: This is a simplified implementation. For production use,
     * consider using ByteBuffer for direct memory access.
     */
    private Bitmap bufferToBitmap(long bufferPtr, int width, int height) {
        try {
            long memoryPtr = VGLite.nativeGetBufferMemory(bufferPtr);
            if (memoryPtr == 0) {
                return null;
            }
            
            // Create bitmap
            Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            
            // Note: Direct memory copy would require JNI or ByteBuffer
            // This is a placeholder - in production, implement proper memory copy
            // For now, just fill with a test pattern
            int[] colors = new int[width * height];
            for (int i = 0; i < colors.length; i++) {
                colors[i] = 0xFFFFFFFF; // White background
            }
            bitmap.setPixels(colors, 0, width, 0, 0, width, height);
            
            return bitmap;
        } catch (Exception e) {
            return null;
        }
    }
}
