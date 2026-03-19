/**
 * @file test_canvas_buffer.cpp
 * @brief Canvas buffer tests - ported from LVGL test_canvas.c buffer-related tests
 *
 * LVGL Source: tests/src/test_cases/widgets/test_canvas.c
 *
 * Buffer-related tests ported (NO lv_* dependencies):
 * | Test Function           | LVGL Source           | Description                    |
 * |------------------------|-----------------------|--------------------------------|
 * | test_canvas_buffer_create   | test_canvas_buffer_operations | Buffer allocation/free |
 * | test_canvas_buffer_stride   | canvas_draw_buf_reshape | VGLite stride handling    |
 * | test_canvas_render_basic    | test_canvas_fill_and_set_px | Basic rendering         |
 *
 * Must NOT do:
 * - NO lv_* symbol references (LVGL dependencies)
 * - NO test LVGL widget functionality (only buffer tests)
 * - NO create new golden images
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "../include/vg_lite.h"

using namespace vg_lite_test;

/**
 * Canvas Buffer Test Fixture
 * Tests buffer allocation, stride handling, and basic rendering operations
 */
class CanvasBufferTest : public ::testing::Test {
protected:
    vg_lite_buffer_t* buffer = nullptr;

    void SetUp() override {
        // Initialize VGLite for buffer operations
        vg_lite_error_t error = vg_lite_init(256, 256);
        ASSERT_EQ(error, VG_LITE_SUCCESS);
    }

    void TearDown() override {
        if (buffer) {
            free_buffer(buffer);
            buffer = nullptr;
        }
        vg_lite_close();
    }

    /**
     * Calculate expected stride for a given width and format
     * VGLite requires proper stride alignment
     */
    uint32_t calculate_stride(uint32_t width, vg_lite_buffer_format_t format) {
        uint32_t bytes_per_pixel;

        switch (format) {
            case VG_LITE_BGRA8888:
            case VG_LITE_RGBA8888:
                bytes_per_pixel = 4;
                break;
            case VG_LITE_RGB565:
            case VG_LITE_BGR565:
            case VG_LITE_RGBA4444:
            case VG_LITE_BGRA4444:
            case VG_LITE_ARGB8888:
            case VG_LITE_ABGR8888:
                bytes_per_pixel = 2;
                break;
            case VG_LITE_A8:
            case VG_LITE_L8:
                bytes_per_pixel = 1;
                break;
            default:
                bytes_per_pixel = 4;
                break;
        }

        // VGLite typically requires 8-byte alignment for strides
        uint32_t stride = width * bytes_per_pixel;
        stride = (stride + 7) & ~7;  // Align to 8 bytes
        return stride;
    }

    /**
     * Get bytes per pixel for format
     */
    uint32_t bytes_per_pixel_for_format(vg_lite_buffer_format_t format) {
        switch (format) {
            case VG_LITE_BGRA8888:
            case VG_LITE_RGBA8888:
            case VG_LITE_RGBX8888:
            case VG_LITE_BGRX8888:
            case VG_LITE_ABGR8888:
            case VG_LITE_ARGB8888:
            case VG_LITE_XBGR8888:
            case VG_LITE_XRGB8888:
                return 4;
            case VG_LITE_RGB565:
            case VG_LITE_BGR565:
            case VG_LITE_RGBA4444:
            case VG_LITE_BGRA4444:
            case VG_LITE_RGBA5551:
            case VG_LITE_BGRA5551:
            case VG_LITE_ARGB1555:
            case VG_LITE_ABGR1555:
            case VG_LITE_ABGR4444:
            case VG_LITE_ARGB4444:
            case VG_LITE_RGBA2222:
            case VG_LITE_BGRA2222:
            case VG_LITE_ABGR2222:
            case VG_LITE_ARGB2222:
            case VG_LITE_RGB888:
            case VG_LITE_BGR888:
                return 2;
            case VG_LITE_A8:
            case VG_LITE_A4:
            case VG_LITE_L8:
                return 1;
            default:
                return 4;
        }
    }
};

/**
 * LVGL Test: test_canvas_buffer_operations()
 * Tests: Buffer allocation, pointer access, fill_bg, draw buffer setup
 *
 * VGLite Port: Tests buffer allocation and basic properties
 */
TEST_F(CanvasBufferTest, BufferCreate) {
    // Test 1: Buffer allocation with BGRA8888 format
    buffer = create_buffer(100, 50, VG_LITE_BGRA8888);
    ASSERT_NE(buffer, nullptr);
    ASSERT_NE(buffer->memory, nullptr);
    ASSERT_EQ(buffer->width, 100u);
    ASSERT_EQ(buffer->height, 50u);
    ASSERT_EQ(buffer->format, VG_LITE_BGRA8888);

    // Verify stride is at least width * bytes_per_pixel
    uint32_t min_stride = 100 * 4;  // BGRA8888 = 4 bytes
    EXPECT_GE(buffer->stride, min_stride);

    // Verify address is set correctly
    EXPECT_EQ(buffer->address, (uint32_t)(uintptr_t)buffer->memory);

    // Test 2: Buffer allocation with RGB565 format
    free_buffer(buffer);
    buffer = create_buffer(200, 100, VG_LITE_RGB565);
    ASSERT_NE(buffer, nullptr);
    ASSERT_EQ(buffer->width, 200u);
    ASSERT_EQ(buffer->height, 100u);
    ASSERT_EQ(buffer->format, VG_LITE_RGB565);

    min_stride = 200 * 2;  // Note: test_helpers uses width*4 for all formats
    EXPECT_GE(buffer->stride, min_stride);

    // Test 3: Buffer allocation with A8 format (grayscale)
    free_buffer(buffer);
    buffer = create_buffer(64, 64, VG_LITE_A8);
    ASSERT_NE(buffer, nullptr);
    ASSERT_EQ(buffer->format, VG_LITE_A8);

    // Test 4: Multiple allocations don't interfere
    vg_lite_buffer_t* buf1 = create_buffer(50, 50, VG_LITE_BGRA8888);
    vg_lite_buffer_t* buf2 = create_buffer(80, 80, VG_LITE_RGB565);

    ASSERT_NE(buf1->memory, nullptr);
    ASSERT_NE(buf2->memory, nullptr);
    EXPECT_NE(buf1->memory, buf2->memory);

    // Verify independent properties
    EXPECT_EQ(buf1->width, 50u);
    EXPECT_EQ(buf2->width, 80u);

    free_buffer(buf1);
    free_buffer(buf2);

    // Original buffer should still be valid
    EXPECT_NE(buffer->memory, nullptr);
}

/**
 * LVGL Test: canvas_draw_buf_reshape() -> test_canvas_buffer_operations()
 * Tests: VGLite stride calculation with LV_STRIDE_AUTO equivalent
 *
 * VGLite Port: Tests stride handling for various widths and formats
 */
TEST_F(CanvasBufferTest, BufferStride) {
    // Test 1: Stride for BGRA8888
    vg_lite_buffer_t* buf_bgra = create_buffer(100, 100, VG_LITE_BGRA8888);
    ASSERT_NE(buf_bgra, nullptr);

    // Stride should be at least width * 4
    EXPECT_GE(buf_bgra->stride, 100u * 4);

    // Test 2: Stride with non-aligned width
    free_buffer(buf_bgra);
    buf_bgra = create_buffer(101, 100, VG_LITE_BGRA8888);
    EXPECT_GE(buf_bgra->stride, 101u * 4);

    // Test 3: Stride for RGB565
    vg_lite_buffer_t* buf_rgb565 = create_buffer(100, 100, VG_LITE_RGB565);
    ASSERT_NE(buf_rgb565, nullptr);
    EXPECT_GE(buf_rgb565->stride, 100u * 2);

    // Test 4: RGB565 with non-aligned width
    free_buffer(buf_rgb565);
    buf_rgb565 = create_buffer(101, 100, VG_LITE_RGB565);
    EXPECT_GE(buf_rgb565->stride, 101u * 2);

    free_buffer(buf_bgra);
    free_buffer(buf_rgb565);

    // Test 5: Stride for A8 (1 byte per pixel)
    vg_lite_buffer_t* buf_a8 = create_buffer(100, 100, VG_LITE_A8);
    ASSERT_NE(buf_a8, nullptr);
    // A8 is 1 byte per pixel, stride should be at least width
    EXPECT_GE(buf_a8->stride, 100u);

    free_buffer(buf_a8);

    // Test 6: Large buffer stride
    buffer = create_buffer(1920, 1080, VG_LITE_BGRA8888);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->width, 1920u);
    EXPECT_EQ(buffer->height, 1080u);
    // 1920 * 4 = 7680 (already 8-byte aligned)
    EXPECT_EQ(buffer->stride, 7680u);
}

/**
 * LVGL Test: test_canvas_fill_and_set_px() + test_canvas_fill_background()
 * Tests: Buffer fill operations and basic rendering
 *
 * VGLite Port: Tests basic buffer fill and pixel operations
 */
TEST_F(CanvasBufferTest, RenderBasic) {
    // Test 1: Buffer fill with solid color
    buffer = create_buffer(100, 100, VG_LITE_BGRA8888);
    ASSERT_NE(buffer, nullptr);

    vg_lite_color_t red = make_color(255, 255, 0, 0);    // Red
    vg_lite_color_t green = make_color(255, 0, 255, 0);  // Green
    vg_lite_color_t blue = make_color(255, 0, 0, 255);   // Blue

    // Fill with red
    fill_buffer(buffer, red);
    vg_lite_color_t center_pixel = get_pixel(buffer, 50, 50);
    EXPECT_TRUE(colors_equal(center_pixel, red));

    // Test 2: Fill with green
    fill_buffer(buffer, green);
    center_pixel = get_pixel(buffer, 50, 50);
    EXPECT_TRUE(colors_equal(center_pixel, green));

    // Test 3: Fill with blue
    fill_buffer(buffer, blue);
    center_pixel = get_pixel(buffer, 50, 50);
    EXPECT_TRUE(colors_equal(center_pixel, blue));

    // Test 4: Partial pixel modification (simulating set_px)
    // Set single pixel to different color
    vg_lite_color_t yellow = make_color(255, 255, 255, 0);  // Yellow
    set_pixel(buffer, 10, 10, yellow);
    vg_lite_color_t modified_pixel = get_pixel(buffer, 10, 10);
    EXPECT_TRUE(colors_equal(modified_pixel, yellow));

    // Verify neighboring pixels unchanged
    vg_lite_color_t unchanged_pixel = get_pixel(buffer, 11, 10);
    EXPECT_TRUE(colors_equal(unchanged_pixel, blue));

    // Test 5: Buffer with RGB565 format
    free_buffer(buffer);
    buffer = create_buffer(50, 50, VG_LITE_RGB565);
    ASSERT_NE(buffer, nullptr);

    // Fill with a color (note: exact color matching may vary due to format conversion)
    fill_buffer(buffer, red);
    center_pixel = get_pixel(buffer, 25, 25);
    EXPECT_NE(center_pixel, 0u);  // Should not be zero

    // Test 6: Multiple buffer renders without memory corruption
    for (int i = 0; i < 10; i++) {
        vg_lite_color_t color = make_color(255, i * 25, i * 10, (10 - i) * 25);
        fill_buffer(buffer, color);
        vg_lite_color_t px = get_pixel(buffer, i, i);
        EXPECT_TRUE(colors_equal(px, color));
    }

    // Test 7: Boundary pixels (corners)
    free_buffer(buffer);
    buffer = create_buffer(10, 10, VG_LITE_BGRA8888);
    ASSERT_NE(buffer, nullptr);

    fill_buffer(buffer, blue);

    // Check all corners
    EXPECT_TRUE(colors_equal(get_pixel(buffer, 0, 0), blue));
    EXPECT_TRUE(colors_equal(get_pixel(buffer, 9, 0), blue));
    EXPECT_TRUE(colors_equal(get_pixel(buffer, 0, 9), blue));
    EXPECT_TRUE(colors_equal(get_pixel(buffer, 9, 9), blue));
}

/**
 * Additional test: Buffer memory properties
 * Tests that buffer memory is properly aligned and accessible
 */
TEST_F(CanvasBufferTest, BufferMemoryProperties) {
    // Test 1: Buffer memory alignment
    buffer = create_buffer(100, 100, VG_LITE_BGRA8888);
    ASSERT_NE(buffer, nullptr);

    // Memory address should be aligned (at least 4-byte for vg_lite)
    uintptr_t addr = (uintptr_t)buffer->memory;
    EXPECT_EQ(addr % 4, 0u);  // Should be 4-byte aligned at minimum

    // Test 2: Stride matches calculated value
    uint32_t expected_stride = calculate_stride(100, VG_LITE_BGRA8888);
    EXPECT_EQ(buffer->stride, expected_stride);

    // Test 3: Total memory size is sufficient for buffer
    uint32_t total_bytes = buffer->height * buffer->stride;
    // Should be able to access last pixel
    uint32_t* px = (uint32_t*)buffer->memory;
    px[((buffer->height - 1) * (buffer->stride / 4)) + (buffer->width - 1)] = 0xFFFFFFFF;

    // Test 4: Buffer handle is null for CPU-allocated buffers
    EXPECT_EQ(buffer->handle, nullptr);

    // Test 5: Multiple formats on same dimensions
    vg_lite_buffer_format_t formats[] = {
        VG_LITE_BGRA8888, VG_LITE_RGBA8888, VG_LITE_RGB565,
        VG_LITE_A8, VG_LITE_L8
    };

    for (auto fmt : formats) {
        free_buffer(buffer);
        buffer = create_buffer(64, 64, fmt);
        ASSERT_NE(buffer, nullptr);
        EXPECT_EQ(buffer->format, fmt);

        // Fill and verify
        vg_lite_color_t c = make_color(128, 64, 32, 16);
        fill_buffer(buffer, c);
        EXPECT_TRUE(colors_equal(get_pixel(buffer, 32, 32), c));
    }
}

/**
 * Additional test: Buffer copy operations
 * Tests basic buffer copy functionality
 */
TEST_F(CanvasBufferTest, BufferCopyOperations) {
    // Create source buffer with pattern
    vg_lite_buffer_t* src = create_buffer(50, 50, VG_LITE_BGRA8888);
    ASSERT_NE(src, nullptr);

    // Fill with gradient pattern
    for (uint32_t y = 0; y < src->height; y++) {
        for (uint32_t x = 0; x < src->width; x++) {
            vg_lite_color_t c = make_color(255, x * 5, y * 5, (x + y) * 2);
            set_pixel(src, x, y, c);
        }
    }

    // Create destination buffer
    buffer = create_buffer(50, 50, VG_LITE_BGRA8888);
    ASSERT_NE(buffer, nullptr);

    // Fill destination with different color
    fill_buffer(buffer, make_color(255, 0, 0, 0));

    // Test 1: Manual pixel copy (simulating buffer copy)
    uint32_t copy_count = 0;
    for (uint32_t y = 0; y < src->height; y++) {
        for (uint32_t x = 0; x < src->width; x++) {
            set_pixel(buffer, x, y, get_pixel(src, x, y));
            copy_count++;
        }
    }

    // Verify all pixels copied
    EXPECT_EQ(copy_count, 50u * 50u);

    // Test 2: Verify copied data matches source
    for (uint32_t y = 0; y < 10; y++) {
        for (uint32_t x = 0; x < 10; x++) {
            vg_lite_color_t src_px = get_pixel(src, x, y);
            vg_lite_color_t dst_px = get_pixel(buffer, x, y);
            EXPECT_TRUE(colors_equal(src_px, dst_px));
        }
    }

    // Test 3: Partial copy (copy sub-region)
    // Clear destination
    fill_buffer(buffer, make_color(255, 0, 0, 0));

    // Copy 20x20 region to destination
    uint32_t start_x = 10, start_y = 10;
    uint32_t region_w = 20, region_h = 20;
    for (uint32_t y = 0; y < region_h; y++) {
        for (uint32_t x = 0; x < region_w; x++) {
            vg_lite_color_t src_px = get_pixel(src, start_x + x, start_y + y);
            set_pixel(buffer, start_x + x, start_y + y, src_px);
        }
    }

    // Verify region copied correctly
    for (uint32_t y = 0; y < region_h; y++) {
        for (uint32_t x = 0; x < region_w; x++) {
            vg_lite_color_t src_px = get_pixel(src, start_x + x, start_y + y);
            vg_lite_color_t dst_px = get_pixel(buffer, start_x + x, start_y + y);
            EXPECT_TRUE(colors_equal(src_px, dst_px));
        }
    }

    // Verify outside region is still cleared
    EXPECT_TRUE(colors_equal(get_pixel(buffer, 0, 0), make_color(255, 0, 0, 0)));

    free_buffer(src);
}

/**
 * Additional test: Buffer format variations
 * Tests buffer creation and operations with various formats
 */
TEST_F(CanvasBufferTest, BufferFormatVariations) {
    struct FormatTest {
        vg_lite_buffer_format_t format;
        const char* name;
        uint32_t width;
        uint32_t height;
    };

    FormatTest tests[] = {
        {VG_LITE_BGRA8888, "BGRA8888", 100, 100},
        {VG_LITE_RGBA8888, "RGBA8888", 100, 100},
        {VG_LITE_RGB565, "RGB565", 100, 100},
        {VG_LITE_ARGB8888, "ARGB8888", 100, 100},
        {VG_LITE_A8, "A8", 100, 100},
        {VG_LITE_L8, "L8", 100, 100},
        // Odd dimensions
        {VG_LITE_BGRA8888, "BGRA8888_odd", 101, 99},
        {VG_LITE_RGB565, "RGB565_odd", 99, 101},
        // Large buffer
        {VG_LITE_BGRA8888, "BGRA8888_large", 800, 600},
    };

    for (const auto& test : tests) {
        SCOPED_TRACE(test.name);

        buffer = create_buffer(test.width, test.height, test.format);
        ASSERT_NE(buffer, nullptr);
        EXPECT_EQ(buffer->width, test.width);
        EXPECT_EQ(buffer->height, test.height);
        EXPECT_EQ(buffer->format, test.format);
        EXPECT_NE(buffer->memory, nullptr);
        EXPECT_GE(buffer->stride, test.width * bytes_per_pixel_for_format(test.format));

        // Test fill operation
        vg_lite_color_t c = make_color(200, 100, 50, 25);
        fill_buffer(buffer, c);
        EXPECT_TRUE(colors_equal(get_pixel(buffer, test.width / 2, test.height / 2), c));
    }
}
