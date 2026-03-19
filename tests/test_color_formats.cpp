/**
 * @file test_color_formats.cpp
 * @brief Color format tests for vg_lite_buffer_t
 * 
 * Ported from LVGL's test_render_to_*.c files:
 * - test_render_to_argb8888.c
 * - test_render_to_rgb565.c
 * - test_render_to_l8.c
 * - test_render_to_argb4444.c
 * 
 * Tests vg_lite_buffer_t format creation and basic operations.
 * NO LVGL dependencies - uses vg_lite API directly.
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "../include/vg_lite.h"

using namespace vg_lite_test;

/**
 * Helper: Get bytes per pixel for a format
 */
static int get_bpp(vg_lite_buffer_format_t format) {
    switch (format) {
        case VG_LITE_BGRA8888:
        case VG_LITE_RGBA8888:
        case VG_LITE_ARGB8888:
        case VG_LITE_ABGR8888:
        case VG_LITE_RGBX8888:
        case VG_LITE_BGRX8888:
        case VG_LITE_XRGB8888:
        case VG_LITE_XBGR8888:
            return 4;
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
        case VG_LITE_BGRA5551:
        case VG_LITE_RGBA5551:
        case VG_LITE_ARGB1555:
        case VG_LITE_ABGR1555:
        case VG_LITE_BGRA4444:
        case VG_LITE_RGBA4444:
        case VG_LITE_ARGB4444:
        case VG_LITE_ABGR4444:
        case VG_LITE_AYUY2:
            return 2;
        case VG_LITE_L8:
        case VG_LITE_A8:
        case VG_LITE_A4:
            return 1;
        default:
            return 4;  // Default to 32-bit
    }
}

/**
 * Helper: Create buffer with specific format
 */
static vg_lite_buffer_t* create_format_buffer(uint32_t width, uint32_t height, 
                                               vg_lite_buffer_format_t format) {
    vg_lite_buffer_t* buffer = new vg_lite_buffer_t();
    std::memset(buffer, 0, sizeof(vg_lite_buffer_t));
    
    buffer->width = width;
    buffer->height = height;
    buffer->format = format;
    
    int bpp = get_bpp(format);
    buffer->stride = width * bpp;
    
    buffer->memory = std::calloc(height * buffer->stride, 1);
    buffer->address = (uint32_t)(uintptr_t)buffer->memory;
    buffer->handle = nullptr;
    buffer->tiled = VG_LITE_LINEAR;
    
    return buffer;
}

/**
 * Helper: Free format-specific buffer
 */
static void free_format_buffer(vg_lite_buffer_t* buffer) {
    if (buffer) {
        if (buffer->memory) {
            std::free(buffer->memory);
        }
        delete buffer;
    }
}

/**
 * Helper: Set pixel in ARGB8888 format
 */
static void set_pixel_argb8888(vg_lite_buffer_t* buf, uint32_t x, uint32_t y, 
                                uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    if (!buf || !buf->memory || x >= (uint32_t)buf->width || y >= (uint32_t)buf->height) return;
    uint32_t* px = (uint32_t*)buf->memory;
    px[y * (buf->stride / 4) + x] = (a << 24) | (r << 16) | (g << 8) | b;
}

/**
 * Helper: Get pixel from ARGB8888 format
 */
static void get_pixel_argb8888(vg_lite_buffer_t* buf, uint32_t x, uint32_t y,
                                uint8_t* a, uint8_t* r, uint8_t* g, uint8_t* b) {
    if (!buf || !buf->memory || x >= (uint32_t)buf->width || y >= (uint32_t)buf->height) {
        *a = *r = *g = *b = 0;
        return;
    }
    uint32_t* px = (uint32_t*)buf->memory;
    uint32_t val = px[y * (buf->stride / 4) + x];
    *a = (val >> 24) & 0xFF;
    *r = (val >> 16) & 0xFF;
    *g = (val >> 8) & 0xFF;
    *b = val & 0xFF;
}

/**
 * Helper: Set pixel in RGB565 format
 */
static void set_pixel_rgb565(vg_lite_buffer_t* buf, uint32_t x, uint32_t y,
                              uint8_t r, uint8_t g, uint8_t b) {
    if (!buf || !buf->memory || x >= (uint32_t)buf->width || y >= (uint32_t)buf->height) return;
    uint16_t* px = (uint16_t*)buf->memory;
    // RGB565: 5 bits R, 6 bits G, 5 bits B
    uint16_t val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    px[y * (buf->stride / 2) + x] = val;
}

/**
 * Helper: Get pixel from RGB565 format
 */
static void get_pixel_rgb565(vg_lite_buffer_t* buf, uint32_t x, uint32_t y,
                              uint8_t* r, uint8_t* g, uint8_t* b) {
    if (!buf || !buf->memory || x >= (uint32_t)buf->width || y >= (uint32_t)buf->height) {
        *r = *g = *b = 0;
        return;
    }
    uint16_t* px = (uint16_t*)buf->memory;
    uint16_t val = px[y * (buf->stride / 2) + x];
    *r = (val >> 8) & 0xF8;
    *g = (val >> 3) & 0xFC;
    *b = (val << 3) & 0xF8;
}

/**
 * Helper: Set pixel in L8 format (grayscale)
 */
static void set_pixel_l8(vg_lite_buffer_t* buf, uint32_t x, uint32_t y, uint8_t l) {
    if (!buf || !buf->memory || x >= (uint32_t)buf->width || y >= (uint32_t)buf->height) return;
    uint8_t* px = (uint8_t*)buf->memory;
    px[y * buf->stride + x] = l;
}

/**
 * Helper: Get pixel from L8 format
 */
static uint8_t get_pixel_l8(vg_lite_buffer_t* buf, uint32_t x, uint32_t y) {
    if (!buf || !buf->memory || x >= (uint32_t)buf->width || y >= (uint32_t)buf->height) return 0;
    uint8_t* px = (uint8_t*)buf->memory;
    return px[y * buf->stride + x];
}

/**
 * Helper: Set pixel in ARGB4444 format
 */
static void set_pixel_argb4444(vg_lite_buffer_t* buf, uint32_t x, uint32_t y,
                                uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    if (!buf || !buf->memory || x >= (uint32_t)buf->width || y >= (uint32_t)buf->height) return;
    uint16_t* px = (uint16_t*)buf->memory;
    // ARGB4444: 4 bits each
    uint16_t val = ((a & 0xF0) << 8) | ((r & 0xF0) << 4) | (g & 0xF0) | (b >> 4);
    px[y * (buf->stride / 2) + x] = val;
}

/**
 * Helper: Get pixel from ARGB4444 format
 */
static void get_pixel_argb4444(vg_lite_buffer_t* buf, uint32_t x, uint32_t y,
                                uint8_t* a, uint8_t* r, uint8_t* g, uint8_t* b) {
    if (!buf || !buf->memory || x >= (uint32_t)buf->width || y >= (uint32_t)buf->height) {
        *a = *r = *g = *b = 0;
        return;
    }
    uint16_t* px = (uint16_t*)buf->memory;
    uint16_t val = px[y * (buf->stride / 2) + x];
    *a = (val >> 8) & 0xF0;
    *r = (val >> 4) & 0xF0;
    *g = val & 0xF0;
    *b = (val << 4) & 0xF0;
}

// ============================================================================
// Test Fixtures
// ============================================================================

class ColorFormatTest : public ::testing::Test {
protected:
    void SetUp() override {
        vg_lite_error_t error = vg_lite_init(256, 256);
        ASSERT_EQ(error, VG_LITE_SUCCESS);
    }
    
    void TearDown() override {
        vg_lite_close();
    }
};

// ============================================================================
// ARGB8888 Format Tests
// ============================================================================

TEST_F(ColorFormatTest, ARGB8888_BufferCreation) {
    vg_lite_buffer_t* buffer = create_format_buffer(100, 100, VG_LITE_ARGB8888);
    
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->width, 100);
    EXPECT_EQ(buffer->height, 100);
    EXPECT_EQ(buffer->format, VG_LITE_ARGB8888);
    EXPECT_EQ(buffer->stride, 400);  // 100 * 4 bytes
    EXPECT_NE(buffer->memory, nullptr);
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, ARGB8888_PixelReadWrite) {
    vg_lite_buffer_t* buffer = create_format_buffer(10, 10, VG_LITE_ARGB8888);
    ASSERT_NE(buffer, nullptr);
    
    // Test pixel write/read
    set_pixel_argb8888(buffer, 0, 0, 255, 255, 0, 0);      // Red
    set_pixel_argb8888(buffer, 1, 0, 128, 0, 255, 0);      // Semi-transparent green
    set_pixel_argb8888(buffer, 5, 5, 255, 0, 0, 255);      // Full blue
    
    uint8_t a, r, g, b;
    
    get_pixel_argb8888(buffer, 0, 0, &a, &r, &g, &b);
    EXPECT_EQ(a, 255);
    EXPECT_EQ(r, 255);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);
    
    get_pixel_argb8888(buffer, 1, 0, &a, &r, &g, &b);
    EXPECT_EQ(a, 128);
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 255);
    EXPECT_EQ(b, 0);
    
    get_pixel_argb8888(buffer, 5, 5, &a, &r, &g, &b);
    EXPECT_EQ(a, 255);
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 255);
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, ARGB8888_FillAndClear) {
    vg_lite_buffer_t* buffer = create_format_buffer(50, 50, VG_LITE_ARGB8888);
    ASSERT_NE(buffer, nullptr);
    
    // Fill with white
    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 50; x++) {
            set_pixel_argb8888(buffer, x, y, 255, 255, 255, 255);
        }
    }
    
    // Verify fill
    uint8_t a, r, g, b;
    get_pixel_argb8888(buffer, 25, 25, &a, &r, &g, &b);
    EXPECT_EQ(a, 255);
    EXPECT_EQ(r, 255);
    EXPECT_EQ(g, 255);
    EXPECT_EQ(b, 255);
    
    free_format_buffer(buffer);
}

// ============================================================================
// RGB565 Format Tests
// ============================================================================

TEST_F(ColorFormatTest, RGB565_BufferCreation) {
    vg_lite_buffer_t* buffer = create_format_buffer(100, 100, VG_LITE_RGB565);
    
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->width, 100);
    EXPECT_EQ(buffer->height, 100);
    EXPECT_EQ(buffer->format, VG_LITE_RGB565);
    EXPECT_EQ(buffer->stride, 200);  // 100 * 2 bytes
    EXPECT_NE(buffer->memory, nullptr);
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, RGB565_PixelReadWrite) {
    vg_lite_buffer_t* buffer = create_format_buffer(10, 10, VG_LITE_RGB565);
    ASSERT_NE(buffer, nullptr);
    
    // Test pixel write/read (note: RGB565 has reduced precision)
    set_pixel_rgb565(buffer, 0, 0, 255, 0, 0);      // Red
    set_pixel_rgb565(buffer, 1, 0, 0, 255, 0);      // Green
    set_pixel_rgb565(buffer, 2, 0, 0, 0, 255);      // Blue
    set_pixel_rgb565(buffer, 5, 5, 255, 255, 255);  // White
    
    uint8_t r, g, b;
    
    get_pixel_rgb565(buffer, 0, 0, &r, &g, &b);
    // RGB565 has 5-bit precision for R (248 = 0xF8)
    EXPECT_EQ(r, 0xF8);  // 255 -> 248 (quantized to 5 bits)
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);
    
    get_pixel_rgb565(buffer, 1, 0, &r, &g, &b);
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0xFC);  // 255 -> 252 (quantized to 6 bits)
    EXPECT_EQ(b, 0);
    
    get_pixel_rgb565(buffer, 2, 0, &r, &g, &b);
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0xF8);  // 255 -> 248 (quantized to 5 bits)
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, RGB565_StrideCalculation) {
    // Test that stride is correctly calculated for odd widths
    vg_lite_buffer_t* buffer = create_format_buffer(99, 50, VG_LITE_RGB565);
    
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->stride, 198);  // 99 * 2 bytes
    
    // Test pixel access at edges
    set_pixel_rgb565(buffer, 0, 0, 255, 0, 0);
    set_pixel_rgb565(buffer, 98, 49, 0, 255, 0);
    
    uint8_t r, g, b;
    get_pixel_rgb565(buffer, 0, 0, &r, &g, &b);
    EXPECT_EQ(r, 0xF8);
    
    get_pixel_rgb565(buffer, 98, 49, &r, &g, &b);
    EXPECT_EQ(g, 0xFC);
    
    free_format_buffer(buffer);
}

// ============================================================================
// L8 (Grayscale) Format Tests
// ============================================================================

TEST_F(ColorFormatTest, L8_BufferCreation) {
    vg_lite_buffer_t* buffer = create_format_buffer(100, 100, VG_LITE_L8);
    
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->width, 100);
    EXPECT_EQ(buffer->height, 100);
    EXPECT_EQ(buffer->format, VG_LITE_L8);
    EXPECT_EQ(buffer->stride, 100);  // 100 * 1 byte
    EXPECT_NE(buffer->memory, nullptr);
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, L8_PixelReadWrite) {
    vg_lite_buffer_t* buffer = create_format_buffer(10, 10, VG_LITE_L8);
    ASSERT_NE(buffer, nullptr);
    
    // Test grayscale values
    set_pixel_l8(buffer, 0, 0, 255);  // White
    set_pixel_l8(buffer, 1, 0, 128);  // Mid-gray
    set_pixel_l8(buffer, 2, 0, 0);    // Black
    set_pixel_l8(buffer, 5, 5, 64);   // Dark gray
    
    EXPECT_EQ(get_pixel_l8(buffer, 0, 0), 255);
    EXPECT_EQ(get_pixel_l8(buffer, 1, 0), 128);
    EXPECT_EQ(get_pixel_l8(buffer, 2, 0), 0);
    EXPECT_EQ(get_pixel_l8(buffer, 5, 5), 64);
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, L8_GradientPattern) {
    vg_lite_buffer_t* buffer = create_format_buffer(256, 10, VG_LITE_L8);
    ASSERT_NE(buffer, nullptr);
    
    // Create horizontal gradient
    for (int x = 0; x < 256; x++) {
        for (int y = 0; y < 10; y++) {
            set_pixel_l8(buffer, x, y, (uint8_t)x);
        }
    }
    
    // Verify gradient
    for (int x = 0; x < 256; x++) {
        EXPECT_EQ(get_pixel_l8(buffer, x, 5), (uint8_t)x);
    }
    
    free_format_buffer(buffer);
}

// ============================================================================
// ARGB4444 Format Tests
// ============================================================================

TEST_F(ColorFormatTest, ARGB4444_BufferCreation) {
    vg_lite_buffer_t* buffer = create_format_buffer(100, 100, VG_LITE_ARGB4444);
    
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->width, 100);
    EXPECT_EQ(buffer->height, 100);
    EXPECT_EQ(buffer->format, VG_LITE_ARGB4444);
    EXPECT_EQ(buffer->stride, 200);  // 100 * 2 bytes
    EXPECT_NE(buffer->memory, nullptr);
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, ARGB4444_PixelReadWrite) {
    vg_lite_buffer_t* buffer = create_format_buffer(10, 10, VG_LITE_ARGB4444);
    ASSERT_NE(buffer, nullptr);
    
    // Test pixel write/read (note: ARGB4444 has 4-bit precision per channel)
    set_pixel_argb4444(buffer, 0, 0, 255, 255, 0, 0);      // Red with full alpha
    set_pixel_argb4444(buffer, 1, 0, 128, 0, 255, 0);      // Semi-transparent green
    set_pixel_argb4444(buffer, 2, 0, 255, 0, 0, 255);      // Blue with full alpha
    
    uint8_t a, r, g, b;
    
    get_pixel_argb4444(buffer, 0, 0, &a, &r, &g, &b);
    // 4-bit precision means 255 -> 240 (0xF0)
    EXPECT_EQ(a, 0xF0);
    EXPECT_EQ(r, 0xF0);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);
    
    get_pixel_argb4444(buffer, 1, 0, &a, &r, &g, &b);
    EXPECT_EQ(a, 0x80);  // 128 -> 128 (fits in high nibble)
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0xF0);
    EXPECT_EQ(b, 0);
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, ARGB4444_AlphaBlending) {
    vg_lite_buffer_t* buffer = create_format_buffer(10, 10, VG_LITE_ARGB4444);
    ASSERT_NE(buffer, nullptr);
    
    // Test various alpha levels
    set_pixel_argb4444(buffer, 0, 0, 255, 255, 0, 0);   // Opaque red
    set_pixel_argb4444(buffer, 1, 0, 192, 255, 0, 0);   // 75% red
    set_pixel_argb4444(buffer, 2, 0, 128, 255, 0, 0);   // 50% red
    set_pixel_argb4444(buffer, 3, 0, 64, 255, 0, 0);    // 25% red
    set_pixel_argb4444(buffer, 4, 0, 0, 255, 0, 0);     // Transparent
    
    uint8_t a, r, g, b;
    
    get_pixel_argb4444(buffer, 0, 0, &a, &r, &g, &b);
    EXPECT_EQ(a, 0xF0);
    
    get_pixel_argb4444(buffer, 4, 0, &a, &r, &g, &b);
    EXPECT_EQ(a, 0);
    
    free_format_buffer(buffer);
}

// ============================================================================
// Cross-Format Tests
// ============================================================================

TEST_F(ColorFormatTest, FormatSizes) {
    // Verify that format sizes are correct
    EXPECT_EQ(get_bpp(VG_LITE_ARGB8888), 4);
    EXPECT_EQ(get_bpp(VG_LITE_BGRA8888), 4);
    EXPECT_EQ(get_bpp(VG_LITE_RGB565), 2);
    EXPECT_EQ(get_bpp(VG_LITE_BGR565), 2);
    EXPECT_EQ(get_bpp(VG_LITE_L8), 1);
    EXPECT_EQ(get_bpp(VG_LITE_A8), 1);
    EXPECT_EQ(get_bpp(VG_LITE_ARGB4444), 2);
    EXPECT_EQ(get_bpp(VG_LITE_BGRA4444), 2);
}

TEST_F(ColorFormatTest, MultipleFormatsCoexist) {
    // Create buffers of different formats simultaneously
    vg_lite_buffer_t* buf_argb = create_format_buffer(50, 50, VG_LITE_ARGB8888);
    vg_lite_buffer_t* buf_rgb565 = create_format_buffer(50, 50, VG_LITE_RGB565);
    vg_lite_buffer_t* buf_l8 = create_format_buffer(50, 50, VG_LITE_L8);
    vg_lite_buffer_t* buf_argb4444 = create_format_buffer(50, 50, VG_LITE_ARGB4444);
    
    ASSERT_NE(buf_argb, nullptr);
    ASSERT_NE(buf_rgb565, nullptr);
    ASSERT_NE(buf_l8, nullptr);
    ASSERT_NE(buf_argb4444, nullptr);
    
    // Write to each buffer
    set_pixel_argb8888(buf_argb, 25, 25, 255, 255, 0, 0);
    set_pixel_rgb565(buf_rgb565, 25, 25, 255, 0, 0);
    set_pixel_l8(buf_l8, 25, 25, 128);
    set_pixel_argb4444(buf_argb4444, 25, 25, 255, 0, 255, 0);
    
    // Verify each buffer independently
    uint8_t a, r, g, b;
    get_pixel_argb8888(buf_argb, 25, 25, &a, &r, &g, &b);
    EXPECT_EQ(a, 255);
    EXPECT_EQ(r, 255);
    
    get_pixel_rgb565(buf_rgb565, 25, 25, &r, &g, &b);
    EXPECT_EQ(r, 0xF8);
    
    EXPECT_EQ(get_pixel_l8(buf_l8, 25, 25), 128);
    
    get_pixel_argb4444(buf_argb4444, 25, 25, &a, &r, &g, &b);
    EXPECT_EQ(a, 0xF0);
    EXPECT_EQ(g, 0xF0);
    
    free_format_buffer(buf_argb);
    free_format_buffer(buf_rgb565);
    free_format_buffer(buf_l8);
    free_format_buffer(buf_argb4444);
}

// ============================================================================
// Boundary Tests
// ============================================================================

TEST_F(ColorFormatTest, ARGB8888_BoundaryAccess) {
    vg_lite_buffer_t* buffer = create_format_buffer(100, 100, VG_LITE_ARGB8888);
    ASSERT_NE(buffer, nullptr);
    
    // Test corner pixels
    set_pixel_argb8888(buffer, 0, 0, 255, 255, 0, 0);           // Top-left
    set_pixel_argb8888(buffer, 99, 0, 255, 0, 255, 0);          // Top-right
    set_pixel_argb8888(buffer, 0, 99, 255, 0, 0, 255);          // Bottom-left
    set_pixel_argb8888(buffer, 99, 99, 255, 255, 255, 255);     // Bottom-right
    
    uint8_t a, r, g, b;
    
    get_pixel_argb8888(buffer, 0, 0, &a, &r, &g, &b);
    EXPECT_EQ(r, 255);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);
    
    get_pixel_argb8888(buffer, 99, 99, &a, &r, &g, &b);
    EXPECT_EQ(r, 255);
    EXPECT_EQ(g, 255);
    EXPECT_EQ(b, 255);
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, RGB565_BoundaryAccess) {
    vg_lite_buffer_t* buffer = create_format_buffer(100, 100, VG_LITE_RGB565);
    ASSERT_NE(buffer, nullptr);
    
    // Test corner pixels
    set_pixel_rgb565(buffer, 0, 0, 255, 0, 0);
    set_pixel_rgb565(buffer, 99, 99, 0, 255, 0);
    
    uint8_t r, g, b;
    
    get_pixel_rgb565(buffer, 0, 0, &r, &g, &b);
    EXPECT_EQ(r, 0xF8);
    
    get_pixel_rgb565(buffer, 99, 99, &r, &g, &b);
    EXPECT_EQ(g, 0xFC);
    
    free_format_buffer(buffer);
}

TEST_F(ColorFormatTest, L8_BoundaryAccess) {
    vg_lite_buffer_t* buffer = create_format_buffer(100, 100, VG_LITE_L8);
    ASSERT_NE(buffer, nullptr);
    
    // Test corner pixels
    set_pixel_l8(buffer, 0, 0, 255);
    set_pixel_l8(buffer, 99, 99, 128);
    
    EXPECT_EQ(get_pixel_l8(buffer, 0, 0), 255);
    EXPECT_EQ(get_pixel_l8(buffer, 99, 99), 128);
    
    free_format_buffer(buffer);
}
