package com.example.vglite;

import android.app.Activity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;

/**
 * MainActivity - Entry point for VGLite Android example with EGL/OpenGL ES
 * 
 * This activity demonstrates VGLite rendering on Android using native EGL
 * with color cycling animation at ~60 FPS.
 * Colors cycle: Red -> Green -> Blue -> Yellow -> Cyan
 */
public class MainActivity extends Activity implements SurfaceHolder.Callback {

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
        
        // Create SurfaceView and set callback
        surfaceView = new SurfaceView(this);
        surfaceView.getHolder().addCallback(this);
        setContentView(surfaceView);
        
        Log.d(TAG, "MainActivity created");
    }
    
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG, "Surface created");
        
        // Get screen dimensions
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        int width = metrics.widthPixels;
        int height = metrics.heightPixels;
        
        Log.d(TAG, "Screen dimensions: " + width + "x" + height);
        
        // Create renderer with screen dimensions
        renderer = new VGLiteRenderer(width, height);
        
        // Initialize VGLite with EGL using the surface
        if (renderer.initWithSurface(holder.getSurface())) {
            Log.d(TAG, "VGLite initialized with EGL");
            // Start color cycling animation
            renderer.renderStart();
            Log.d(TAG, "Render thread started");
        } else {
            Log.e(TAG, "Failed to initialize VGLite");
        }
    }
    
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(TAG, "Surface destroyed");
        
        if (renderer != null) {
            // Stop the render thread
            renderer.renderStop();
            Log.d(TAG, "Render thread stopped");
        }
    }
    
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        Log.d(TAG, "Surface changed: " + w + "x" + h);
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
    }
    
    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");
        
        // Stop rendering when pausing
        if (renderer != null) {
            renderer.renderStop();
        }
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        
        // Clean up all native resources
        if (renderer != null) {
            renderer.cleanup();
            renderer = null;
            Log.d(TAG, "VGLite resources cleaned up");
        }
    }
}
