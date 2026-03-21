/**
 * @file test_helpers.h
 * @brief Test helper utilities for VGLite tests
 */

#ifndef VG_LITE_TEST_HELPERS_H
#define VG_LITE_TEST_HELPERS_H

#include "../include/vg_lite.h"
#include "image_compare.h"
#include <gtest/gtest.h>
#include <cmath>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <vector>
#ifdef _WIN32
#include <malloc.h>
#include <windows.h>
#endif

// External globals from test_main.cpp
extern bool g_dump_enabled;
extern std::string g_output_dir;

namespace vg_lite_test {

/**
 * GL Context Manager - creates a hidden OpenGL context for GL backend tests
 * On Windows, uses WGL to create a hidden window with GL context
 */
class GLContextManager {
public:
    static GLContextManager& instance() {
        static GLContextManager inst;
        return inst;
    }
    
    bool initialize() {
#if VG_LITE_RENDER_BACKEND == 2 && defined(_WIN32)
        if (m_initialized) return m_context != nullptr;
        m_initialized = true;
        
        // Step 1: Create a temporary window/context to load WGL extensions
        HINSTANCE hInst = GetModuleHandle(nullptr);
        
        WNDCLASSA tempWc = {};
        tempWc.lpfnWndProc = DefWindowProcA;
        tempWc.hInstance = hInst;
        tempWc.lpszClassName = "VGLiteTempGLClass";
        RegisterClassA(&tempWc);
        
        HWND tempHwnd = CreateWindowExA(0, "VGLiteTempGLClass", "Temp", 
            0, 0, 0, 1, 1, nullptr, nullptr, hInst, nullptr);
        if (!tempHwnd) return false;
        
        HDC tempHdc = GetDC(tempHwnd);
        
        // Basic pixel format for temp context
        PIXELFORMATDESCRIPTOR tempPfd = {};
        tempPfd.nSize = sizeof(tempPfd);
        tempPfd.nVersion = 1;
        tempPfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
        tempPfd.iPixelType = PFD_TYPE_RGBA;
        tempPfd.cColorBits = 32;
        
        int tempPf = ChoosePixelFormat(tempHdc, &tempPfd);
        SetPixelFormat(tempHdc, tempPf, &tempPfd);
        
        HGLRC tempContext = wglCreateContext(tempHdc);
        if (!tempContext) {
            DestroyWindow(tempHwnd);
            return false;
        }
        
        wglMakeCurrent(tempHdc, tempContext);
        
        // Step 2: Load WGL extension functions
        typedef int (WINAPI *wglChoosePixelFormatARB_type)(HDC, const int*, const float*, UINT, int*, UINT*);
        typedef HGLRC (WINAPI *wglCreateContextAttribsARB_type)(HDC, HGLRC, const int*);
        
        wglChoosePixelFormatARB_type wglChoosePixelFormatARB = 
            (wglChoosePixelFormatARB_type)wglGetProcAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB_type wglCreateContextAttribsARB = 
            (wglCreateContextAttribsARB_type)wglGetProcAddress("wglCreateContextAttribsARB");
        
        if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
            // Extensions not available, fall back to legacy context
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(tempContext);
            DestroyWindow(tempHwnd);
            
            // Use temp context as-is (legacy)
            m_hInstance = hInst;
            m_hwnd = tempHwnd;
            m_hdc = tempHdc;
            m_hglrc = tempContext;
            m_context = tempContext;
            return true;
        }
        
        // Step 3: Choose proper pixel format for OpenGL 3.3
        int pixelAttribs[] = {
            0x2001, 1,  // WGL_DRAW_TO_WINDOW_ARB
            0x2003, 1,  // WGL_SUPPORT_OPENGL_ARB
            0x2010, 1,  // WGL_DOUBLE_BUFFER_ARB
            0x2011, 32, // WGL_COLOR_BITS_ARB
            0x2012, 24, // WGL_DEPTH_BITS_ARB
            0x2013, 8,  // WGL_STENCIL_BITS_ARB
            0x2014, 0,  // WGL_PIXEL_TYPE_ARB (0 = RGBA)
            0
        };
        
        int pixelFormat = 0;
        UINT numFormats = 0;
        BOOL choseFormat = wglChoosePixelFormatARB(tempHdc, pixelAttribs, nullptr, 1, &pixelFormat, &numFormats);
        
        if (!choseFormat || numFormats == 0) {
            // No suitable pixel format found, fall back to legacy
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(tempContext);
            ReleaseDC(tempHwnd, tempHdc);
            DestroyWindow(tempHwnd);
            UnregisterClassA("VGLiteTempGLClass", hInst);
            
            // Fall back to legacy context creation
            m_hInstance = hInst;
            
            WNDCLASSA wc = {};
            wc.lpfnWndProc = DefWindowProcA;
            wc.hInstance = m_hInstance;
            wc.lpszClassName = "VGLiteTestGLClass";
            if (!RegisterClassA(&wc)) return false;
            
            m_hwnd = CreateWindowExA(0, "VGLiteTestGLClass", "VGLiteTestGL",
                0, 0, 0, 1, 1, nullptr, nullptr, m_hInstance, nullptr);
            if (!m_hwnd) return false;
            
            m_hdc = GetDC(m_hwnd);
            if (!m_hdc) return false;
            
            PIXELFORMATDESCRIPTOR pfd = {};
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            
            int pf = ChoosePixelFormat(m_hdc, &pfd);
            if (!pf || !SetPixelFormat(m_hdc, pf, &pfd)) return false;
            
            m_hglrc = wglCreateContext(m_hdc);
            if (!m_hglrc) return false;
            
            if (!wglMakeCurrent(m_hdc, m_hglrc)) {
                wglDeleteContext(m_hglrc);
                m_hglrc = nullptr;
                return false;
            }
            
            m_context = m_hglrc;
            return true;
        }
        
        // Step 4: Cleanup temp window
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(tempContext);
        ReleaseDC(tempHwnd, tempHdc);
        DestroyWindow(tempHwnd);
        UnregisterClassA("VGLiteTempGLClass", hInst);
        
        // Step 5: Create real window with proper pixel format
        m_hInstance = hInst;
        
        WNDCLASSA wc = {};
        wc.lpfnWndProc = DefWindowProcA;
        wc.hInstance = m_hInstance;
        wc.lpszClassName = "VGLiteTestGLClass";
        if (!RegisterClassA(&wc)) return false;
        
        m_hwnd = CreateWindowExA(0, "VGLiteTestGLClass", "VGLiteTestGL",
            0, 0, 0, 1, 1, nullptr, nullptr, m_hInstance, nullptr);
        if (!m_hwnd) return false;
        
        m_hdc = GetDC(m_hwnd);
        if (!m_hdc) return false;
        
        SetPixelFormat(m_hdc, pixelFormat, &tempPfd);
        
        // Step 6: Create OpenGL 3.3 context
        int contextAttribs[] = {
            0x2091, 3,  // WGL_CONTEXT_MAJOR_VERSION_ARB
            0x2092, 3,  // WGL_CONTEXT_MINOR_VERSION_ARB
            0x9126, 0x00000001,  // WGL_CONTEXT_CORE_PROFILE_BIT_ARB
            0
        };
        
        m_hglrc = wglCreateContextAttribsARB(m_hdc, nullptr, contextAttribs);
        if (!m_hglrc) return false;
        
        if (!wglMakeCurrent(m_hdc, m_hglrc)) {
            wglDeleteContext(m_hglrc);
            m_hglrc = nullptr;
            return false;
        }
        
        m_context = m_hglrc;
        m_isModernContext = true;  // Mark as modern GL 3.3+ context
        return true;
#else
        m_initialized = true;
        return true;
#endif
    }
    
    void cleanup() {
#if VG_LITE_RENDER_BACKEND == 2 && defined(_WIN32)
        if (m_hglrc) {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(m_hglrc);
            m_hglrc = nullptr;
        }
        if (m_hwnd && m_hdc) {
            ReleaseDC(m_hwnd, m_hdc);
            m_hdc = nullptr;
        }
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        m_context = nullptr;
#endif
    }
    
    bool hasContext() const {
#if VG_LITE_RENDER_BACKEND == 2 && defined(_WIN32)
        return m_context != nullptr;
#else
        return true;
#endif
    }
    
    // Check if we have OpenGL 3.3+ (required by ThorVG)
    bool hasGL33() const {
#if VG_LITE_RENDER_BACKEND == 2 && defined(_WIN32)
        // If we don't have a modern context, we don't have GL 3.3
        return m_isModernContext;
#else
        return true;
#endif
    }
    
private:
    GLContextManager() = default;
    ~GLContextManager() { cleanup(); }
    
    bool m_initialized = false;
    void* m_context = nullptr;
    bool m_isModernContext = false;  // True if using OpenGL 3.3+ context
    
#if defined(_WIN32)
    HINSTANCE m_hInstance = nullptr;
    HWND m_hwnd = nullptr;
    HDC m_hdc = nullptr;
    HGLRC m_hglrc = nullptr;
#endif
};

/**
 * Check if GL context is available (for GL backend tests)
 * On Windows, checks if wglGetCurrentContext returns non-null
 */
inline bool hasGLContext() {
#if VG_LITE_RENDER_BACKEND == 2
    #ifdef _WIN32
        // Dynamically check for GL context
        typedef void* (*WGLGETCURRENTCONTEXTPROC)();
        static WGLGETCURRENTCONTEXTPROC wglGetCurrentContext = nullptr;
        static bool initialized = false;
        if (!initialized) {
            HMODULE opengl32 = LoadLibraryA("opengl32.dll");
            if (opengl32) {
                wglGetCurrentContext = (WGLGETCURRENTCONTEXTPROC)GetProcAddress(opengl32, "wglGetCurrentContext");
            }
            initialized = true;
        }
        return wglGetCurrentContext && wglGetCurrentContext() != nullptr;
    #else
        // On non-Windows platforms, assume context is available for now
        return true;
    #endif
#else
    // SW backend doesn't need GL context
    return true;
#endif
}

/**
 * Check if rendering tests should be skipped (GL backend without proper GL 3.3+ context)
 */
inline bool shouldSkipRenderingTests() {
#if VG_LITE_RENDER_BACKEND == 2
    // ThorVG GL backend requires OpenGL 3.3+
    return !GLContextManager::instance().hasGL33();
#else
    return false;
#endif
}

/**
 * Get the directory containing the test executable
 * On Windows, uses GetModuleFileNameW for Unicode support
 * @return Directory path with trailing separator
 */
inline std::string get_exe_directory() {
#ifdef _WIN32
    wchar_t wpath[MAX_PATH * 2];  // Use larger buffer for long paths
    HMODULE hModule = GetModuleHandleW(nullptr);
    if (GetModuleFileNameW(hModule, wpath, MAX_PATH * 2) == 0) {
        // Fallback to current directory if we can't get exe path
        return "./";
    }
    
    // Convert wide string to narrow string
    char path[MAX_PATH * 2];
    WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path, MAX_PATH * 2, nullptr, nullptr);
    
    // Find the last separator and truncate to get directory
    std::string result(path);
    size_t last_sep = result.find_last_of("\\/");
    if (last_sep != std::string::npos) {
        result = result.substr(0, last_sep + 1);
    } else {
        result = "./";
    }
    return result;
#else
    // On non-Windows platforms, use /proc/self/exe or argv[0]
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        std::string result(path);
        size_t last_sep = result.find_last_of('/');
        if (last_sep != std::string::npos) {
            result = result.substr(0, last_sep + 1);
        }
        return result;
    }
    return "./";
#endif
}

/**
 * Get reference images directory path (relative to project root)
 * Works from build directory or project root
 */
inline std::string get_ref_imgs_dir() {
    // Try multiple common locations
    // 1. From project root: tests/ref_imgs/
    // 2. From build directory: ../tests/ref_imgs/ or ../../tests/ref_imgs/
    
    std::vector<std::string> search_paths = {
        "tests/ref_imgs",           // From project root
        "../tests/ref_imgs",        // From build/
        "../../tests/ref_imgs",     // From build/Debug/ or build/Release/
        "../../../tests/ref_imgs",  // From build/tests/Debug/
    };
    
    for (const auto& path : search_paths) {
        // Check if path exists by trying to open a known file
        std::string test_file = path + "/draw/vector_draw_shapes.lp64.png";
        FILE* f = fopen(test_file.c_str(), "rb");
        if (f) {
            fclose(f);
            return path;
        }
        // Also try without lp64
        test_file = path + "/draw/vector_draw_shapes.lp32.png";
        f = fopen(test_file.c_str(), "rb");
        if (f) {
            fclose(f);
            return path;
        }
    }
    
    // Default fallback
    return "tests/ref_imgs";
}

/**
 * Get full path to a golden image
 * @param relative_path Path relative to ref_imgs directory (e.g., "draw/vector_draw_shapes")
 * @return Full path to golden image (tries .lp64.png, .lp32.png, .png)
 */
inline std::string get_golden_path(const std::string& relative_path) {
    std::string base_dir = get_ref_imgs_dir();
    
    // If path already has .png extension, use as-is
    if (relative_path.find(".png") != std::string::npos) {
        return base_dir + "/" + relative_path;
    }
    
    // Try variants in order
    std::vector<std::string> variants = {
        base_dir + "/" + relative_path + ".lp64.png",
        base_dir + "/" + relative_path + ".lp32.png",
        base_dir + "/" + relative_path + ".png",
    };
    
    for (const auto& path : variants) {
        FILE* f = fopen(path.c_str(), "rb");
        if (f) {
            fclose(f);
            return path;
        }
    }
    
    // Return default even if doesn't exist (will fail in comparison)
    return variants[0];
}

/**
 * Create a simple test buffer
 * Uses aligned_alloc for proper 64-byte alignment required by vg_lite_draw
 */
inline vg_lite_buffer_t* create_buffer(uint32_t width, uint32_t height, 
                                        vg_lite_buffer_format_t format = VG_LITE_BGRA8888) {
    vg_lite_buffer_t* buffer = new vg_lite_buffer_t();
    std::memset(buffer, 0, sizeof(vg_lite_buffer_t));
    
    buffer->width = width;
    buffer->height = height;
    buffer->format = format;
    buffer->tiled = VG_LITE_LINEAR;
    
    // Calculate stride: bytes per pixel * width
    // Determine bytes per pixel based on format
    int bytes_per_pixel = 4;  // default for 32-bit formats
    switch(format) {
        case VG_LITE_BGRA8888:
        case VG_LITE_RGBA8888:
        case VG_LITE_ABGR8888:
        case VG_LITE_ARGB8888:
            bytes_per_pixel = 4;
            break;
        case VG_LITE_RGB565:
        case VG_LITE_ARGB4444:
        case VG_LITE_BGRA5551:
        case VG_LITE_BGRA4444:
        case VG_LITE_BGR565:
        case VG_LITE_ARGB1555:
            bytes_per_pixel = 2;
            break;
        case VG_LITE_L8:
        case VG_LITE_A8:
            bytes_per_pixel = 1;
            break;
        default:
            bytes_per_pixel = 4;
    }
    
    // Calculate stride with proper alignment for all formats
    // Stride must be aligned to 64 bytes per VG_LITE_BUF_ADDR_ALIGN
    buffer->stride = width * bytes_per_pixel;
    // Ensure stride is at least 4-byte aligned for 16-bit formats
    if (bytes_per_pixel < 4) {
        buffer->stride = (buffer->stride + 3) & ~3;  // Round up to 4-byte boundary
    }
    
    // Set default image mode for blit operations
    buffer->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    buffer->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    
    // Allocate aligned memory (64-byte alignment as per VG_LITE requirements)
    size_t size = height * buffer->stride;
#if defined(_WIN32)
    buffer->memory = _aligned_malloc(size, 64);
#elif defined(__ANDROID__)
    buffer->memory = memalign(64, size);
#else
    buffer->memory = aligned_alloc(64, size);
#endif
    if (!buffer->memory) {
        delete buffer;
        return nullptr;
    }
    std::memset(buffer->memory, 0, size);
    
    buffer->address = (uint32_t)(uintptr_t)buffer->memory;
    // handle is for GPU usage, keep as nullptr for CPU-allocated buffers
    
    return buffer;
}

/**
 * Free a test buffer
 */
inline void free_buffer(vg_lite_buffer_t* buffer) {
    if (buffer) {
        // Free memory allocated with _aligned_malloc/aligned_alloc
        if (buffer->memory) {
#ifdef _WIN32
            _aligned_free(buffer->memory);
#else
            free(buffer->memory);
#endif
        }
        delete buffer;
    }
}

/**
 * Create a rectangle path (correct format for ThorVG)
 * Format: [op(4bytes)][x(4bytes)][y(4bytes)] per command
 */
inline vg_lite_path_t* create_rect_path(float x, float y, float w, float h) {
    vg_lite_path_t* path = new vg_lite_path_t();
    std::memset(path, 0, sizeof(vg_lite_path_t));
    
    path->bounding_box[0] = x;
    path->bounding_box[1] = y;
    path->bounding_box[2] = x + w;
    path->bounding_box[3] = y + h;
    path->quality = VG_LITE_MEDIUM;
    path->format = VG_LITE_FP32;
    
    // MOVE: 4 + 4 + 4 = 12 bytes
    // 4x LINE: 4 * (4 + 4 + 4) = 48 bytes
    // CLOSE: 4 bytes
    // END: 4 bytes
    // Total: 12 + 48 + 4 + 4 = 68 bytes
    int total_bytes = 68;
    uint8_t* data = (uint8_t*)std::malloc(total_bytes);
    uint8_t* ptr = data;
    
    // MOVE to (x, y)
    *(uint32_t*)ptr = VLC_OP_MOVE; ptr += 4;
    *(float*)ptr = x; ptr += 4;
    *(float*)ptr = y; ptr += 4;
    
    // LINE to (x+w, y)
    *(uint32_t*)ptr = VLC_OP_LINE; ptr += 4;
    *(float*)ptr = x + w; ptr += 4;
    *(float*)ptr = y; ptr += 4;
    
    // LINE to (x+w, y+h)
    *(uint32_t*)ptr = VLC_OP_LINE; ptr += 4;
    *(float*)ptr = x + w; ptr += 4;
    *(float*)ptr = y + h; ptr += 4;
    
    // LINE to (x, y) - close the rect explicitly
    *(uint32_t*)ptr = VLC_OP_LINE; ptr += 4;
    *(float*)ptr = x; ptr += 4;
    *(float*)ptr = y; ptr += 4;

    // CLOSE path
    *(uint32_t*)ptr = VLC_OP_CLOSE; ptr += 4;

    // END path
    *(uint32_t*)ptr = VLC_OP_END; ptr += 4;

    path->path = data;
    path->path_length = total_bytes;

    return path;
}

/**
 * Create a rounded rectangle path (simplified - just use regular rectangle)
 * For true rounded rectangles, vg_lite doesn't support border-radius directly
 * so we approximate with a standard rectangle
 */
inline vg_lite_path_t* create_round_rect_path(float x, float y, float w, float h, float radius) {
    // For simplicity, just use standard rectangle
    // vg_lite doesn't support border-radius directly
    // We could implement rounded corners using cubic bezier curves
    // but for the tests, we approximate with a regular rectangle
    return create_rect_path(x, y, w, h);
}

/**
 * Create a circle path using cubic Bezier curves (like LVGL)
 * Uses 4 cubic Beziers to approximate a circle with PATH_KAPPA constant
 * Correct format: [op(4bytes)][coords(4bytes each)]
 */
inline vg_lite_path_t* create_circle_path(float cx, float cy, float radius, int segs = 32) {
    const float PATH_KAPPA = 0.552284f;
    float rx_kappa = radius * PATH_KAPPA;
    float ry_kappa = radius * PATH_KAPPA;
    
    vg_lite_path_t* path = new vg_lite_path_t();
    std::memset(path, 0, sizeof(vg_lite_path_t));
    
    path->bounding_box[0] = cx - radius;
    path->bounding_box[1] = cy - radius;
    path->bounding_box[2] = cx + radius;
    path->bounding_box[3] = cy + radius;
    path->quality = VG_LITE_MEDIUM;
    path->format = VG_LITE_FP32;
    
    // MOVE: 4 (op) + 4 (x) + 4 (y) = 12 bytes
    // 4x CUBIC: 4 * (4 + 4*6) = 4 * 28 = 112 bytes
    // CLOSE: 4 bytes
    // END: 4 bytes
    // Total: 12 + 112 + 4 + 4 = 132 bytes
    int total_bytes = 132;
    uint8_t* data = (uint8_t*)std::malloc(total_bytes);
    uint8_t* ptr = data;
    
    // MOVE to top of circle
    *(uint32_t*)ptr = VLC_OP_MOVE; ptr += 4;
    *(float*)ptr = cx; ptr += 4;
    *(float*)ptr = cy - radius; ptr += 4;
    
    // 4 cubic Bezier curves for the circle
    // Right side (top-right, bottom-right)
    *(uint32_t*)ptr = VLC_OP_CUBIC; ptr += 4;
    *(float*)ptr = cx + rx_kappa;  ptr += 4;  // cp1x
    *(float*)ptr = cy - radius;    ptr += 4;  // cp1y
    *(float*)ptr = cx + radius;    ptr += 4;  // cp2x
    *(float*)ptr = cy - ry_kappa;  ptr += 4;  // cp2y
    *(float*)ptr = cx + radius;    ptr += 4;  // x
    *(float*)ptr = cy;             ptr += 4;  // y
    
    // Bottom-right to bottom-left
    *(uint32_t*)ptr = VLC_OP_CUBIC; ptr += 4;
    *(float*)ptr = cx + radius;    ptr += 4;  // cp1x
    *(float*)ptr = cy + ry_kappa;  ptr += 4;  // cp1y
    *(float*)ptr = cx + rx_kappa;  ptr += 4;  // cp2x
    *(float*)ptr = cy + radius;    ptr += 4;  // cp2y
    *(float*)ptr = cx;             ptr += 4;  // x
    *(float*)ptr = cy + radius;    ptr += 4;  // y
    
    // Bottom-left to top-left
    *(uint32_t*)ptr = VLC_OP_CUBIC; ptr += 4;
    *(float*)ptr = cx - rx_kappa;  ptr += 4;  // cp1x
    *(float*)ptr = cy + radius;    ptr += 4;  // cp1y
    *(float*)ptr = cx - radius;    ptr += 4;  // cp2x
    *(float*)ptr = cy + ry_kappa;  ptr += 4;  // cp2y
    *(float*)ptr = cx - radius;    ptr += 4;  // x
    *(float*)ptr = cy;             ptr += 4;  // y
    
    // Top-left back to start
    *(uint32_t*)ptr = VLC_OP_CUBIC; ptr += 4;
    *(float*)ptr = cx - radius;    ptr += 4;  // cp1x
    *(float*)ptr = cy - ry_kappa;  ptr += 4;  // cp1y
    *(float*)ptr = cx - rx_kappa;  ptr += 4;  // cp2x
    *(float*)ptr = cy - radius;    ptr += 4;  // cp2y
    *(float*)ptr = cx;             ptr += 4;  // x
    *(float*)ptr = cy - radius;    ptr += 4;  // y
    
    // CLOSE path
    *(uint32_t*)ptr = VLC_OP_CLOSE; ptr += 4;
    
    // END
    *(uint32_t*)ptr = VLC_OP_END; ptr += 4;
    
    path->path = data;
    path->path_length = total_bytes;
    return path;
}

/**
 * Free a path
 */
inline void free_path(vg_lite_path_t* path) {
    if (path) {
        if (path->path) std::free(path->path);
        delete path;
    }
}

/**
 * Make BGRA color from ARGB components
 */
inline vg_lite_color_t make_color(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return ((vg_lite_color_t)a << 24) | ((vg_lite_color_t)b << 16) |
           ((vg_lite_color_t)g << 8) | (vg_lite_color_t)r;
}

/**
 * Get pixel from buffer
 */
inline vg_lite_color_t get_pixel(vg_lite_buffer_t* buf, uint32_t x, uint32_t y) {
    if (!buf || !buf->memory || x >= buf->width || y >= buf->height) return 0;
    uint32_t* px = (uint32_t*)buf->memory;
    return px[y * (buf->stride / 4) + x];
}

/**
 * Set pixel in buffer
 */
inline void set_pixel(vg_lite_buffer_t* buf, uint32_t x, uint32_t y, vg_lite_color_t color) {
    if (!buf || !buf->memory || x >= buf->width || y >= buf->height) return;
    uint32_t* px = (uint32_t*)buf->memory;
    px[y * (buf->stride / 4) + x] = color;
}

/**
 * ARGB1555 pixel components (1-bit alpha, 5-bit R/G/B)
 */
struct argb1555_pixel_t {
    uint8_t a;  // 0-1
    uint8_t r;  // 0-31
    uint8_t g;  // 0-31
    uint8_t b;  // 0-31
};

/**
 * Get ARGB1555 pixel from buffer
 * ARGB1555 layout: A(1) | R(5) | G(5) | B(5)
 */
inline argb1555_pixel_t get_pixel_argb1555(vg_lite_buffer_t* buf, uint32_t x, uint32_t y) {
    argb1555_pixel_t px = {0, 0, 0, 0};
    if (!buf || !buf->memory || x >= buf->width || y >= buf->height) return px;
    uint16_t* data = (uint16_t*)buf->memory;
    uint16_t val = data[y * (buf->stride / 2) + x];
    px.a = (val >> 15) & 0x01;
    px.r = (val >> 10) & 0x1F;
    px.g = (val >> 5) & 0x1F;
    px.b = val & 0x1F;
    return px;
}

/**
 * Fill buffer with color
 * Handles different pixel formats correctly
 */
inline void fill_buffer(vg_lite_buffer_t* buf, vg_lite_color_t color) {
    if (!buf || !buf->memory) return;
    
    // Extract ARGB components from color (format: AARRGGBB in make_color)
    uint8_t a = (color >> 24) & 0xFF;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    // Handle different formats
    switch (buf->format) {
        case VG_LITE_ARGB1555:
        case VG_LITE_RGBA5551:
        case VG_LITE_ABGR1555:
        case VG_LITE_BGRA5551: {
            // 16-bit formats: ARGB1555 - A(1) R(5) G(5) B(5)
            uint16_t* px = (uint16_t*)buf->memory;
            uint32_t count = buf->height * (buf->stride / 2);
            uint16_t val;
            if (buf->format == VG_LITE_ARGB1555) {
                // ARGB1555: A(1) R(5) G(5) B(5)
                uint8_t a1 = (a > 127) ? 1 : 0;
                uint8_t r5 = (r * 31) / 255;
                uint8_t g5 = (g * 31) / 255;
                uint8_t b5 = (b * 31) / 255;
                val = (a1 << 15) | (r5 << 10) | (g5 << 5) | b5;
            } else {
                // Default 16-bit fill
                val = 0xFFFF;
            }
            for (uint32_t i = 0; i < count; i++) px[i] = val;
            break;
        }
        
        case VG_LITE_RGB565:
        case VG_LITE_BGR565: {
            // 16-bit RGB565: R(5) G(6) B(5)
            uint16_t* px = (uint16_t*)buf->memory;
            uint32_t count = buf->height * (buf->stride / 2);
            uint8_t r5 = (r * 31) / 255;
            uint8_t g6 = (g * 63) / 255;
            uint8_t b5 = (b * 31) / 255;
            uint16_t val = (r5 << 11) | (g6 << 5) | b5;
            for (uint32_t i = 0; i < count; i++) px[i] = val;
            break;
        }
        
        case VG_LITE_ARGB4444:
        case VG_LITE_BGRA4444:
        case VG_LITE_RGBA4444:
        case VG_LITE_ABGR4444: {
            // 16-bit ARGB4444: A(4) R(4) G(4) B(4)
            uint16_t* px = (uint16_t*)buf->memory;
            uint32_t count = buf->height * (buf->stride / 2);
            uint8_t a4 = a >> 4;
            uint8_t r4 = r >> 4;
            uint8_t g4 = g >> 4;
            uint8_t b4 = b >> 4;
            uint16_t val = (a4 << 12) | (r4 << 8) | (g4 << 4) | b4;
            for (uint32_t i = 0; i < count; i++) px[i] = val;
            break;
        }
        
        case VG_LITE_L8:
        case VG_LITE_A8: {
            // 8-bit formats
            uint8_t* px = (uint8_t*)buf->memory;
            uint32_t count = buf->height * buf->stride;
            uint8_t val = (buf->format == VG_LITE_L8) ? r : a;  // L8 uses red, A8 uses alpha
            for (uint32_t i = 0; i < count; i++) px[i] = val;
            break;
        }
        
        default: {
            // 32-bit formats (BGRA8888, etc.)
            uint32_t* px = (uint32_t*)buf->memory;
            uint32_t count = buf->height * (buf->stride / 4);
            for (uint32_t i = 0; i < count; i++) px[i] = color;
            break;
        }
    }
}

/**
 * Compare two colors with tolerance
 */
inline bool colors_equal(vg_lite_color_t c1, vg_lite_color_t c2, uint8_t tol = 5) {
    uint8_t a1 = (c1 >> 24) & 0xFF, b1 = (c1 >> 16) & 0xFF;
    uint8_t g1 = (c1 >> 8) & 0xFF, r1 = c1 & 0xFF;
    uint8_t a2 = (c2 >> 24) & 0xFF, b2 = (c2 >> 16) & 0xFF;
    uint8_t g2 = (c2 >> 8) & 0xFF, r2 = c2 & 0xFF;
    return std::abs(a1 - a2) <= tol && std::abs(b1 - b2) <= tol &&
           std::abs(g1 - g2) <= tol && std::abs(r1 - r2) <= tol;
}

/**
 * Save buffer to JSON
 * JSON format: {"test_name": "...", "width": N, "height": N, "format": "...", "timestamp": "..."}
 */
inline bool save_buffer_to_json(vg_lite_buffer_t* buf, const std::string& path, const std::string& test_name) {
    if (!buf) return false;
    
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return false;
    
    // Get format string
    std::string format_str;
    switch (buf->format) {
        case VG_LITE_BGRA8888: format_str = "BGRA8888"; break;
        case VG_LITE_RGB565: format_str = "RGB565"; break;
        case VG_LITE_L8: format_str = "L8"; break;
        default: format_str = "UNKNOWN"; break;
    }
    
    // Get current timestamp
    time_t now = time(nullptr);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    // Write JSON
    fprintf(f, "{\n");
    fprintf(f, "  \"test_name\": \"%s\",\n", test_name.c_str());
    fprintf(f, "  \"width\": %u,\n", buf->width);
    fprintf(f, "  \"height\": %u,\n", buf->height);
    fprintf(f, "  \"format\": \"%s\",\n", format_str.c_str());
    fprintf(f, "  \"timestamp\": \"%s\"\n", timestamp);
    fprintf(f, "}\n");
    
    fclose(f);
    return true;
}

/**
 * Save buffer to PNG with logging
 */
inline bool save_buffer_to_png(vg_lite_buffer_t* buf, const std::string& path) {
    if (!buf || !buf->memory) {
        std::cerr << "\n[ERROR] save_buffer_to_png: Invalid buffer (null)" << std::endl;
        return false;
    }
    bool result = saveBufferToPng(path, (const uint8_t*)buf->memory, buf->width, buf->height, buf->format, buf->stride);
    if (result) {
        std::cout << "\n[OUTPUT] Saved PNG: " << path << std::endl;
    } else {
        std::cerr << "\n[ERROR] Failed to save PNG: " << path << std::endl;
    }
    return result;
}

/**
 * Compare with golden image
 */
inline bool compare_with_golden(vg_lite_buffer_t* buf, const std::string& golden_path, 
                                 double tolerance = 0.05) {
    if (!buf || !buf->memory) return false;
    
    // Extract relative path (remove any ../ prefix)
    std::string rel_path = golden_path;
    if (rel_path.find("../") == 0) {
        rel_path = rel_path.substr(3);  // Skip "../"
    }
    
    // Build search paths - try multiple locations
    std::vector<std::string> search_paths;
    std::string exe_dir = get_exe_directory();
    
    // From build/windows/sw/bin/Release/ -> ../../tests/
    search_paths.push_back(exe_dir + "../../tests/" + rel_path);
    search_paths.push_back(exe_dir + "../tests/" + rel_path);
    // Current directory
    search_paths.push_back(rel_path);
    search_paths.push_back("tests/" + rel_path);
    search_paths.push_back("../../tests/" + rel_path);
    
    Image golden;
    for (const auto& path : search_paths) {
        golden = loadImage(path);
        if (golden.valid()) break;
    }
    
    // If path doesn't already have extension, try variants
    if (!golden.valid() && golden_path.find(".png") == std::string::npos) {
        for (const auto& base_path : search_paths) {
            // Try .lp64.png first (64-bit systems)
            golden = loadImage(base_path + ".lp64.png");
            if (golden.valid()) break;
            
            // Try .lp32.png (32-bit systems)
            golden = loadImage(base_path + ".lp32.png");
            if (golden.valid()) break;
            
            // Try plain .png
            golden = loadImage(base_path + ".png");
            if (golden.valid()) break;
        }
    }
    
    if (!golden.valid()) return false;
    
    Image actual = bufferToImage((const uint8_t*)buf->memory, buf->width, buf->height, 
                                  buf->format, buf->stride);
    CompareResult res = compareImages(actual, golden, tolerance);
    
    if (!res.match) saveImage("test_output_mismatch.png", actual);
    return res.match;
}

/**
 * Get comparison message
 */
inline std::string get_comparison_message(vg_lite_buffer_t* buf, const std::string& golden_path,
                                           double tolerance = 0.05) {
    if (!buf || !buf->memory) return "Invalid buffer";
    
    // Extract relative path (remove any ../ prefix)
    std::string rel_path = golden_path;
    if (rel_path.find("../") == 0) {
        rel_path = rel_path.substr(3);  // Skip "../"
    }
    
    // Build search paths - try multiple locations
    std::vector<std::string> search_paths;
    std::string exe_dir = get_exe_directory();
    
    // From build/windows/sw/bin/Release/ -> ../../tests/
    search_paths.push_back(exe_dir + "../../tests/" + rel_path);
    search_paths.push_back(exe_dir + "../tests/" + rel_path);
    // Current directory
    search_paths.push_back(rel_path);
    search_paths.push_back("tests/" + rel_path);
    search_paths.push_back("../../tests/" + rel_path);
    
    Image golden;
    std::string loaded_path;
    for (const auto& path : search_paths) {
        golden = loadImage(path);
        if (golden.valid()) {
            loaded_path = path;
            break;
        }
    }
    
    // If path doesn't already have extension, try variants
    if (!golden.valid() && golden_path.find(".png") == std::string::npos) {
        for (const auto& base_path : search_paths) {
            // Try .lp64.png first (64-bit systems)
            golden = loadImage(base_path + ".lp64.png");
            if (golden.valid()) { loaded_path = base_path + ".lp64.png"; break; }
            
            // Try .lp32.png (32-bit systems)
            golden = loadImage(base_path + ".lp32.png");
            if (golden.valid()) { loaded_path = base_path + ".lp32.png"; break; }
            
            // Try plain .png
            golden = loadImage(base_path + ".png");
            if (golden.valid()) { loaded_path = base_path + ".png"; break; }
        }
    }
    
    if (!golden.valid()) return "Failed to load golden: " + rel_path;
    
    Image actual = bufferToImage((const uint8_t*)buf->memory, buf->width, buf->height, 
                                  buf->format, buf->stride);
    return compareImages(actual, golden, tolerance).message;
}

/**
 * Base test fixture with dump support
 */
class VGLiteTest : public ::testing::Test {
protected:
    vg_lite_buffer_t* dump_buffer = nullptr;
    std::string dump_test_name;
    
    void SetUp() override {
        // Set up code before each test
    }
    
    void TearDown() override {
        // Handle dump if enabled
        if (g_dump_enabled && dump_buffer) {
            std::string base_name = dump_test_name.empty() ? "test_output" : dump_test_name;
            std::string png_path = g_output_dir + "/" + base_name + ".png";
            std::string json_path = g_output_dir + "/" + base_name + ".json";
            
            save_buffer_to_png(dump_buffer, png_path);
            save_buffer_to_json(dump_buffer, json_path, base_name);
            
            std::cout << "\n[DUMP] Saved: " << png_path << " and " << json_path << std::endl;
        }
    }
    
    // Helper method for derived tests to request a dump
    void RequestDump(vg_lite_buffer_t* buffer, const std::string& test_name) {
        dump_buffer = buffer;
        dump_test_name = test_name;
    }
};

} // namespace vg_lite_test

#endif // VG_LITE_TEST_HELPERS_H
