/**
 * @file hello_window.c
 * @brief Win32 fullscreen window example using VGLite SW backend
 * 
 * Demonstrates:
 * - Creating a fullscreen Win32 window
 * - Querying primary monitor resolution at runtime
 * - Initializing VGLite with SW backend
 * - Clearing screen to a solid color
 * - Handling window close and ESC key
 */

#include <windows.h>
#include <stdio.h>
#include "vg_lite.h"

/* Window class name */
static const char* WINDOW_CLASS_NAME = "VGLiteWindowClass";

/* Window handle */
static HWND g_hwnd = NULL;

/* Running flag */
static int g_running = 1;

/* VGLite buffer */
static vg_lite_buffer_t g_render_buffer;

/* Screen dimensions */
static int g_screen_width = 0;
static int g_screen_height = 0;

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
    g_render_buffer.format = VG_LITE_RGBA8888;
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
 * Clean up VGLite resources
 */
static void cleanup_vglite(void)
{
    vg_lite_free(&g_render_buffer);
    vg_lite_close();
    OutputDebugStringA("VGLite resources cleaned up\n");
}

/**
 * Render a frame
 */
static void render_frame(void)
{
    vg_lite_error_t error;
    
    /* Clear the buffer to a deep red color (ARGB: 0xFF8B0000) */
    /* This demonstrates a solid color fill */
    error = vg_lite_clear(&g_render_buffer, NULL, 0xFF8B0000);
    if (error != VG_LITE_SUCCESS) {
        OutputDebugStringA("vg_lite_clear failed\n");
        return;
    }
    
    /* Wait for GPU to finish */
    error = vg_lite_finish();
    if (error != VG_LITE_SUCCESS) {
        OutputDebugStringA("vg_lite_finish failed\n");
        return;
    }
    
    /* TODO: In a real application, you would blit g_render_buffer.memory 
     * to the screen using GDI, DirectX, or another presentation API.
     * For this SW backend example, we just clear the buffer.
     */
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
        
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            /* Render the frame */
            render_frame();
            
            /* For SW backend, we'd normally copy buffer to screen here.
             * For simplicity, just fill with a solid color using GDI.
             */
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH brush = CreateSolidBrush(RGB(0x8B, 0x00, 0x00));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
            
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
    
    /* Initialize VGLite */
    result = init_vglite();
    if (result != 0) {
        MessageBoxA(NULL, "Failed to initialize VGLite", "Error", MB_OK | MB_ICONERROR);
        DestroyWindow(g_hwnd);
        return 1;
    }
    
    /* Show the window */
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    
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