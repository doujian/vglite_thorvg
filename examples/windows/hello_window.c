/**
 * @file hello_window.c
 * @brief Win32 fullscreen window example using VGLite GL backend
 * 
 * Demonstrates:
 * - Creating a fullscreen Win32 window
 * - Querying primary monitor resolution at runtime
 * - Initializing VGLite with GL backend
 * - Initializing OpenGL with WGL
 * - Color cycling animation at ~60 FPS
 * - Handling window close and ESC key
 */

#include <windows.h>
#include <gl/GL.h>
#include <stdio.h>
#include "vg_lite.h"

/* Window class name */
static const char* WINDOW_CLASS_NAME = "VGLiteWindowClass";

/* Window handle */
static HWND g_hwnd = NULL;

/* WGL context */
static HDC g_hdc = NULL;
static HGLRC g_hglrc = NULL;

/* Running flag */
static int g_running = 1;

/* VGLite buffer */
static vg_lite_buffer_t g_render_buffer;

/* Screen dimensions */
static int g_screen_width = 0;
static int g_screen_height = 0;

/* Color cycling */
static const vg_lite_color_t g_colors[] = {
    0xFFFF0000,  /* Red */
    0xFF00FF00,  /* Green */
    0xFF0000FF,  /* Blue */
    0xFFFFFF00,  /* Yellow */
    0xFF00FFFF   /* Cyan */
};
static int g_color_index = 0;
static int g_frame_count = 0;

/* Error checking macro */
#define CHECK_ERROR(result, msg) \
    do { \
        if ((result) != VG_LITE_SUCCESS) { \
            char err_buf[256]; \
            snprintf(err_buf, sizeof(err_buf), "ERROR: %s (code: %d)\n", msg, (result)); \
            OutputDebugStringA(err_buf); \
            return (int)(result); \
        } \
    } while (0)

/**
 * Initialize OpenGL context using WGL
 */
static int init_opengl(HWND hwnd)
{
    PIXELFORMATDESCRIPTOR pfd = {0};
    int pixel_format;
    
    /* Get device context */
    g_hdc = GetDC(hwnd);
    if (g_hdc == NULL) {
        OutputDebugStringA("Failed to get device context\n");
        return -1;
    }
    
    /* Set up pixel format descriptor */
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    /* Choose pixel format */
    pixel_format = ChoosePixelFormat(g_hdc, &pfd);
    if (pixel_format == 0) {
        OutputDebugStringA("Failed to choose pixel format\n");
        ReleaseDC(hwnd, g_hdc);
        g_hdc = NULL;
        return -1;
    }
    
    /* Set pixel format */
    if (!SetPixelFormat(g_hdc, pixel_format, &pfd)) {
        OutputDebugStringA("Failed to set pixel format\n");
        ReleaseDC(hwnd, g_hdc);
        g_hdc = NULL;
        return -1;
    }
    
    /* Create OpenGL context */
    g_hglrc = wglCreateContext(g_hdc);
    if (g_hglrc == NULL) {
        OutputDebugStringA("Failed to create OpenGL context\n");
        ReleaseDC(hwnd, g_hdc);
        g_hdc = NULL;
        return -1;
    }
    
    /* Make context current */
    if (!wglMakeCurrent(g_hdc, g_hglrc)) {
        OutputDebugStringA("Failed to make OpenGL context current\n");
        wglDeleteContext(g_hglrc);
        g_hglrc = NULL;
        ReleaseDC(hwnd, g_hdc);
        g_hdc = NULL;
        return -1;
    }
    
    OutputDebugStringA("OpenGL context initialized successfully\n");
    return 0;
}

/**
 * Initialize VGLite and create render buffer
 */
static int init_vglite(void)
{
    vg_lite_error_t error;
    
    /* Initialize VGLite with tessellation buffer size */
    /* Use 256x256 tessellation buffer - adjust as needed for complex paths */
    error = vg_lite_init(256, 256);
    CHECK_ERROR(error, "vg_lite_init failed");
    OutputDebugStringA("VGLite initialized successfully\n");
    
    /* Set up render target buffer */
    g_render_buffer.width = g_screen_width;
    g_render_buffer.height = g_screen_height;
    g_render_buffer.format = VG_LITE_BGRA8888;
    g_render_buffer.tiled = VG_LITE_LINEAR;
    
    /* Allocate buffer memory */
    error = vg_lite_allocate(&g_render_buffer);
    CHECK_ERROR(error, "vg_lite_allocate failed");
    
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "Render buffer allocated: %dx%d\n", 
                 g_render_buffer.width, g_render_buffer.height);
        OutputDebugStringA(buf);
    }
    
    return 0;
}

/**
 * Clean up VGLite and OpenGL resources
 */
static void cleanup_vglite(void)
{
    /* Kill the timer */
    if (g_hwnd != NULL) {
        KillTimer(g_hwnd, 1);
    }
    
    vg_lite_free(&g_render_buffer);
    vg_lite_close();
    
    /* Cleanup OpenGL */
    if (g_hglrc != NULL) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hglrc);
        g_hglrc = NULL;
    }
    if (g_hdc != NULL && g_hwnd != NULL) {
        ReleaseDC(g_hwnd, g_hdc);
        g_hdc = NULL;
    }
    
    OutputDebugStringA("VGLite and OpenGL resources cleaned up\n");
}

/**
 * Render a frame
 * 
 * GL backend: ThorVG renders directly to screen via custom FBO blit
 * SW backend: Need to manually blit the buffer using glDrawPixels
 */
static void render_frame(void)
{
    vg_lite_error_t error;
    
    /* Clear the buffer to the current cycling color */
    error = vg_lite_clear(&g_render_buffer, NULL, g_colors[g_color_index]);
    if (error != VG_LITE_SUCCESS) {
        char buf[128];
        snprintf(buf, sizeof(buf), "vg_lite_clear failed: %d\n", error);
        OutputDebugStringA(buf);
        return;
    }
    
    /* Finish rendering */
    error = vg_lite_finish();
    if (error != VG_LITE_SUCCESS) {
        char buf[128];
        snprintf(buf, sizeof(buf), "vg_lite_finish failed: %d\n", error);
        OutputDebugStringA(buf);
        return;
    }
    
#if VG_LITE_RENDER_BACKEND == 1
    /* SW backend: manually blit the rendered buffer to screen */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, g_screen_width, 0, g_screen_height, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glRasterPos2i(0, 0);
    glDrawPixels(g_render_buffer.width, g_render_buffer.height, 
                 GL_BGRA_EXT, GL_UNSIGNED_BYTE, g_render_buffer.memory);
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
#else
    /* GL backend: ThorVG already blitted to screen via custom FBO */
    (void)g_screen_width;
    (void)g_screen_height;
#endif
    
    /* Swap buffers to display the rendered content */
    SwapBuffers(g_hdc);
    
    /* Update frame count and cycle color every 60 frames (~1 second at 60fps) */
    g_frame_count++;
    if (g_frame_count >= 60) {
        g_frame_count = 0;
        g_color_index = (g_color_index + 1) % 5;
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "Color changed to index %d\n", g_color_index);
            OutputDebugStringA(buf);
        }
    }
}

/**
 * Window procedure for handling messages
 */
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CLOSE:
        /* User clicked close button */
        g_running = 0;
        DestroyWindow(hwnd);
        return 0;
        
    case WM_DESTROY:
        /* Window is being destroyed */
        g_running = 0;
        PostQuitMessage(0);
        return 0;
        
    case WM_KEYDOWN:
        /* Handle keyboard input */
        switch (wParam) {
        case VK_ESCAPE:
            /* ESC key pressed - exit */
            g_running = 0;
            PostQuitMessage(0);
            return 0;
        }
        break;
        
    case WM_TIMER:
        /* Timer tick - render frame (includes SwapBuffers) */
        render_frame();
        return 0;
        
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            
            /* Just render - SwapBuffers is handled in render_frame() */
            render_frame();
            
            /* Validate the entire window */
            ValidateRect(hwnd, NULL);
            
            EndPaint(hwnd, &ps);
        }
        return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/**
 * Register the window class
 */
static int register_window_class(HINSTANCE hInstance)
{
    WNDCLASSEXA wc = {0};
    
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassExA(&wc)) {
        OutputDebugStringA("Failed to register window class\n");
        return -1;
    }
    
    return 0;
}

/**
 * Create a fullscreen window
 */
static HWND create_fullscreen_window(HINSTANCE hInstance)
{
    HWND hwnd;
    
    /* Query primary monitor resolution at runtime */
    g_screen_width = GetSystemMetrics(SM_CXSCREEN);
    g_screen_height = GetSystemMetrics(SM_CYSCREEN);
    
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "Screen resolution: %dx%d\n", 
                 g_screen_width, g_screen_height);
        OutputDebugStringA(buf);
    }
    
    /* Create a borderless fullscreen window */
    /* WS_POPUP removes all window chrome (no borders, no title bar) */
    hwnd = CreateWindowExA(
        0,                              /* Extended style */
        WINDOW_CLASS_NAME,              /* Window class */
        "VGLite Hello Window",          /* Window title */
        WS_POPUP,                       /* Style: popup (borderless) */
        0, 0,                           /* Position: top-left */
        g_screen_width, g_screen_height,/* Size: full screen */
        NULL,                           /* Parent window */
        NULL,                           /* Menu */
        hInstance,                      /* Instance handle */
        NULL                            /* Additional data */
    );
    
    if (hwnd == NULL) {
        OutputDebugStringA("Failed to create window\n");
        return NULL;
    }
    
    return hwnd;
}

/**
 * Main entry point
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    int result;
    
    (void)hPrevInstance;  /* Unused */
    (void)lpCmdLine;      /* Unused */
    
    OutputDebugStringA("VGLite Hello Window starting...\n");
    
    /* Register window class */
    result = register_window_class(hInstance);
    if (result != 0) {
        MessageBoxA(NULL, "Failed to register window class", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    /* Create fullscreen window */
    g_hwnd = create_fullscreen_window(hInstance);
    if (g_hwnd == NULL) {
        MessageBoxA(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    /* Initialize OpenGL FIRST - GL backend requires valid GL context before vg_lite_init */
    result = init_opengl(g_hwnd);
    if (result != 0) {
        MessageBoxA(NULL, "Failed to initialize OpenGL", "Error", MB_OK | MB_ICONERROR);
        DestroyWindow(g_hwnd);
        return 1;
    }
    
    /* Initialize VGLite AFTER OpenGL context is created */
    result = init_vglite();
    if (result != 0) {
        MessageBoxA(NULL, "Failed to initialize VGLite", "Error", MB_OK | MB_ICONERROR);
        cleanup_vglite();
        DestroyWindow(g_hwnd);
        return 1;
    }
    
    /* Show the window */
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    
    /* Set up timer for ~60 FPS (16ms interval) */
    SetTimer(g_hwnd, 1, 16, NULL);
    
    OutputDebugStringA("Window created and displayed, entering message loop\n");
    
    /* Message loop */
    while (g_running) {
        if (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            g_running = 0;
        }
    }
    
    /* Cleanup */
    cleanup_vglite();
    DestroyWindow(g_hwnd);
    
    OutputDebugStringA("VGLite Hello Window exiting cleanly\n");
    
    return 0;
}