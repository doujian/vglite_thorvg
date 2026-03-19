package com.example.vglite;

import android.app.Activity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;

/**
 * MainActivity - Entry point for VGLite Android example
 * 
 * This activity demonstrates VGLite rendering on Android using the JNI bridge.
 * The renderer clears the screen to a red color using vg_lite_clear().
 */
public class MainActivity extends Activity {

    private static final String TAG = "VGLiteExample";
    
    private SurfaceView surfaceView;
    private VGLiteRenderer renderer;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // Enable fullscreen mode
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        );
        
        // Get screen dimensions
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        int width = metrics.widthPixels;
        int height = metrics.heightPixels;
        
        Log.d(TAG, "Screen dimensions: " + width + "x" + height);
        
        // Create renderer with screen dimensions
        renderer = new VGLiteRenderer(width, height);
        
        // Initialize VGLite native library
        if (renderer.init()) {
            Log.d(TAG, "VGLite initialized successfully");
            
            // Clear the buffer to red
            if (renderer.clear()) {
                Log.d(TAG, "Buffer cleared to red");
            } else {
                Log.e(TAG, "Failed to clear buffer");
            }
            
            // Render (flush and finish)
            if (renderer.render()) {
                Log.d(TAG, "Render completed");
            } else {
                Log.e(TAG, "Failed to render");
            }
        } else {
            Log.e(TAG, "Failed to initialize VGLite");
        }
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        // Resume rendering if needed
    }
    
    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");
        // Pause rendering if needed
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        
        // Clean up native resources
        if (renderer != null) {
            renderer.cleanup();
            Log.d(TAG, "VGLite resources cleaned up");
        }
    }
}