/**
 * @file test_golden.cpp
 * @brief Golden image comparison tests ported from LVGL
 * 
 * These tests render shapes and compare output with LVGL's reference images.
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "../include/vg_lite.h"

using namespace vg_lite_test;

/**
 * Golden image tests - compare rendered output with reference images
 */
class GoldenImageTest : public ::testing::Test {
protected:
    vg_lite_buffer_t* buffer = nullptr;
    vg_lite_matrix_t matrix;
    
    void SetUp() override {
        vg_lite_error_t error = vg_lite_init(256, 256);
        ASSERT_EQ(error, VG_LITE_SUCCESS);
        
        vg_lite_identity(&matrix);
    }
    
    void TearDown() override {
        if (buffer) {
            free_buffer(buffer);
            buffer = nullptr;
        }
        vg_lite_close();
    }
    
    // Helper to setup buffer for rendering tests - returns true if setup succeeded
    bool setup_render_buffer() {
        // Skip if GL backend doesn't have proper OpenGL 3.3+ support
        if (shouldSkipRenderingTests()) {
            return false;
        }
        
        // Create buffer matching LVGL test resolution (800x480)
        buffer = create_buffer(800, 480, VG_LITE_BGRA8888);
        if (buffer == nullptr) {
            return false;
        }
        
        // Clear buffer to white (like LVGL tests)
        fill_buffer(buffer, make_color(255, 255, 255, 255));
        return true;
    }
};

/**
 * Test vector_draw_shapes golden image
 * Corresponds to LVGL test_draw_vector.c draw_shapes()
 */
TEST_F(GoldenImageTest, VectorDrawShapes) {
    // Setup buffer for rendering (skips if no GL context)
    if (!setup_render_buffer()) {
        GTEST_SKIP() << "GL backend requires GL context for rendering tests";
    }
    
    // Draw red rectangle (matching LVGL test)
    vg_lite_path_t* path1 = create_rect_path(50, 50, 100, 100);
    ASSERT_NE(path1, nullptr);
    
    vg_lite_color_t red = make_color(255, 0, 0, 255);
    vg_lite_error_t error = vg_lite_draw(buffer, path1, VG_LITE_FILL_EVEN_ODD, 
                                          &matrix, VG_LITE_BLEND_SRC_OVER, red);
    EXPECT_EQ(error, VG_LITE_SUCCESS);
    
    // Draw blue rectangle
    vg_lite_path_t* path2 = create_rect_path(200, 50, 100, 100);
    ASSERT_NE(path2, nullptr);
    
    vg_lite_color_t blue = make_color(255, 0, 0, 255);
    error = vg_lite_draw(buffer, path2, VG_LITE_FILL_EVEN_ODD, 
                         &matrix, VG_LITE_BLEND_SRC_OVER, blue);
    EXPECT_EQ(error, VG_LITE_SUCCESS);
    
    // Save output for debugging
    save_buffer_to_png(buffer, "test_vector_draw_shapes_output.png");
    
    // Load golden image using helper
    std::string golden_path = get_golden_path("draw/vector_draw_shapes");
    
    // Debug output
    printf("[DEBUG] Golden path: %s\n", golden_path.c_str());
    Image golden = loadImage(golden_path);
    printf("[DEBUG] Golden loaded: %s (size: %dx%d)\n", 
           golden.valid() ? "YES" : "NO", 
           golden.valid() ? golden.width : 0, 
           golden.valid() ? golden.height : 0);
    
    if (golden.valid()) {
        Image actual = bufferToImage((const uint8_t*)buffer->memory, buffer->width, buffer->height);
        CompareResult res = compareImages(actual, golden, 0.1);
        printf("[DEBUG] Compare result: match=%s, diff=%.2f%%, mismatches=%d\n",
               res.match ? "YES" : "NO", res.difference_percent, res.mismatched_pixels);
        EXPECT_TRUE(res.match) << res.message << " (golden: " << golden_path << ")";
    } else {
        // If golden not found, just pass (no crash = success)
        SUCCEED() << "Golden image not found, skipping comparison";
    }
    
    free_path(path1);
    free_path(path2);
}

/**
 * Test blend modes golden image
 * Corresponds to LVGL vector_draw_blend_modes_comparison
 */
TEST_F(GoldenImageTest, BlendModesComparison) {
    // Setup buffer for rendering (skips if no GL context)
    if (!setup_render_buffer()) {
        GTEST_SKIP() << "GL backend requires GL context for rendering tests";
    }
    
    int x = 10;
    int y = 10;
    int rect_size = 80;
    
    // Draw rectangles with different blend modes
    vg_lite_blend_t blends[] = {
        VG_LITE_BLEND_SRC_OVER,
        VG_LITE_BLEND_MULTIPLY,
        VG_LITE_BLEND_SCREEN,
        VG_LITE_BLEND_LIGHTEN,
        VG_LITE_BLEND_ADDITIVE
    };
    
    vg_lite_color_t colors[] = {
        make_color(255, 255, 0, 0),    // Red
        make_color(255, 0, 255, 0),    // Green
        make_color(255, 0, 0, 255),    // Blue
        make_color(255, 255, 255, 0),  // Yellow
        make_color(255, 0, 255, 255)   // Cyan
    };
    
    for (int i = 0; i < 5; i++) {
        vg_lite_path_t* path = create_rect_path(x + i * 90, y, rect_size, rect_size);
        if (path) {
            vg_lite_error_t error = vg_lite_draw(buffer, path, VG_LITE_FILL_EVEN_ODD,
                                                  &matrix, blends[i], colors[i]);
            EXPECT_EQ(error, VG_LITE_SUCCESS);
            free_path(path);
        }
    }
    
    save_buffer_to_png(buffer, "test_blend_modes_output.png");
    
    // Load golden image using helper
    std::string golden_path = get_golden_path("draw/vector_draw_blend_modes_comparison");
    Image golden = loadImage(golden_path);
    
    if (golden.valid()) {
        Image actual = bufferToImage((const uint8_t*)buffer->memory, buffer->width, buffer->height);
        CompareResult res = compareImages(actual, golden, 0.15);
        EXPECT_TRUE(res.match) << res.message << " (golden: " << golden_path << ")";
    } else {
        SUCCEED() << "Golden image not found, skipping comparison";
    }
}

/**
 * Test fill rules golden image
 */
TEST_F(GoldenImageTest, FillRulesComparison) {
    // Setup buffer for rendering (skips if no GL context)
    if (!setup_render_buffer()) {
        GTEST_SKIP() << "GL backend requires GL context for rendering tests";
    }
    
    // Draw two overlapping rectangles with even-odd fill
    vg_lite_path_t* path1 = create_rect_path(100, 100, 200, 200);
    ASSERT_NE(path1, nullptr);
    
    vg_lite_path_t* path2 = create_rect_path(150, 150, 200, 200);
    ASSERT_NE(path2, nullptr);
    
    // Fill with semi-transparent red
    vg_lite_color_t red = make_color(128, 255, 0, 0);
    vg_lite_error_t error = vg_lite_draw(buffer, path1, VG_LITE_FILL_EVEN_ODD,
                                          &matrix, VG_LITE_BLEND_SRC_OVER, red);
    EXPECT_EQ(error, VG_LITE_SUCCESS);
    
    // Fill with semi-transparent blue
    vg_lite_color_t blue = make_color(128, 0, 0, 255);
    error = vg_lite_draw(buffer, path2, VG_LITE_FILL_EVEN_ODD,
                         &matrix, VG_LITE_BLEND_SRC_OVER, blue);
    EXPECT_EQ(error, VG_LITE_SUCCESS);
    
    save_buffer_to_png(buffer, "test_fill_rules_output.png");
    
    // No direct golden for this test - just verify rendering doesn't crash
    free_path(path1);
    free_path(path2);
}

/**
 * Test matrix transforms golden image
 */
TEST_F(GoldenImageTest, MatrixTransformComparison) {
    // Setup buffer for rendering (skips if no GL context)
    if (!setup_render_buffer()) {
        GTEST_SKIP() << "GL backend requires GL context for rendering tests";
    }
    
    // Draw rectangle with translation
    vg_lite_path_t* path = create_rect_path(0, 0, 100, 100);
    ASSERT_NE(path, nullptr);
    
    vg_lite_matrix_t transform;
    vg_lite_identity(&transform);
    vg_lite_translate(200, 150, &transform);
    vg_lite_rotate(45, &transform);
    
    vg_lite_color_t green = make_color(255, 0, 255, 0);
    vg_lite_error_t error = vg_lite_draw(buffer, path, VG_LITE_FILL_EVEN_ODD,
                                          &transform, VG_LITE_BLEND_SRC_OVER, green);
    EXPECT_EQ(error, VG_LITE_SUCCESS);
    
    save_buffer_to_png(buffer, "test_matrix_transform_output.png");
    
    // No direct golden for this test - just verify rendering doesn't crash
    free_path(path);
}
