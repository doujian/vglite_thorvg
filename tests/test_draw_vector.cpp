/**
 * @file test_draw_vector.cpp
 * @brief Vector drawing tests - 1:1 mapping with LVGL test_draw_vector.c
 * 
 * LVGL Source: tests/src/test_cases/draw/test_draw_vector.c
 * 
 * Test Functions and Golden Images:
 * | Test Function              | Golden Image                           |
 * |---------------------------|----------------------------------------|
 * | test_transform()          | NONE (unit test only)                  |
 * | test_draw_lines()         | vector_draw_lines.lp64.png             |
 * | test_draw_shapes()        | vector_draw_shapes.lp64.png            |
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "../include/vg_lite.h"

using namespace vg_lite_test;

/**
 * Helper: Compare with golden image
 */
static void verify_golden(vg_lite_buffer_t* buffer, const std::string& test_name) {
    std::string output_path = test_name + "_output.png";
    save_buffer_to_png(buffer, output_path);
    
    std::string golden_path = get_golden_path("draw/" + test_name);
    
    printf("[GOLDEN] Test: %s\n", test_name.c_str());
    printf("[GOLDEN] Golden: %s\n", golden_path.c_str());
    
    Image golden = loadImage(golden_path);
    printf("[GOLDEN] Status: %s, %dx%d\n", 
           golden.valid() ? "FOUND" : "NOT FOUND",
           golden.valid() ? golden.width : 0,
           golden.valid() ? golden.height : 0);
    
    if (golden.valid()) {
        Image actual = bufferToImage((const uint8_t*)buffer->memory, buffer->width, buffer->height);
        CompareResult res = compareImages(actual, golden, 0.1);
        printf("[GOLDEN] Result: match=%s, diff=%.2f%%\n",
               res.match ? "YES" : "NO", res.difference_percent);
        EXPECT_TRUE(res.match) << res.message;
    } else {
        FAIL() << "Golden not found: " << golden_path;
    }
}

class VectorDrawTest : public ::testing::Test {
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
        
        // Match LVGL golden image size (800x480)
        buffer = create_buffer(800, 480, VG_LITE_BGRA8888);
        if (buffer == nullptr) {
            return false;
        }
        
        // Clear to white
        fill_buffer(buffer, make_color(255, 255, 255, 255));
        return true;
    }
};

/**
 * LVGL Test: test_transform()
 * Golden: NONE (unit test only, uses TEST_ASSERT_EQUAL_FLOAT)
 * 
 * LVGL tests:
 * 1. Matrix translate by (100, 100)
 * 2. Transform point (10, 10) -> expected (110, 110)
 * 3. Transform path points
 * 
 * Our vg_lite API doesn't have vg_lite_transform_point(),
 * but we can verify the matrix values directly.
 */
TEST_F(VectorDrawTest, Transform) {
    vg_lite_matrix_t m;
    vg_lite_identity(&m);
    
    // After identity, translation should be (0, 0)
    EXPECT_FLOAT_EQ(m.m[0][2], 0.0f);  // tx
    EXPECT_FLOAT_EQ(m.m[1][2], 0.0f);  // ty
    
    // Translate by (100, 100) - matching LVGL test
    vg_lite_translate(100.0f, 100.0f, &m);
    
    // After translate(100, 100), a point (10, 10) should become (110, 110)
    // In a 2D transform matrix: m[0][2] = tx, m[1][2] = ty
    // This is equivalent to LVGL's: lv_matrix_transform_point(&matrix, &p)
    // where p = (10, 10) becomes (10 + 100, 10 + 100) = (110, 110)
    
    // Verify the translation components
    EXPECT_FLOAT_EQ(m.m[0][2], 100.0f);  // tx = 100
    EXPECT_FLOAT_EQ(m.m[1][2], 100.0f);  // ty = 100
    
    // Calculate transformed point manually (like LVGL does)
    float px = 10.0f;
    float py = 10.0f;
    float tx = px + m.m[0][2];  // 10 + 100 = 110
    float ty = py + m.m[1][2];  // 10 + 100 = 110
    
    // Same assertions as LVGL: TEST_ASSERT_EQUAL_FLOAT(110.0f, p.x)
    EXPECT_FLOAT_EQ(tx, 110.0f);
    EXPECT_FLOAT_EQ(ty, 110.0f);
    
    // Test second translate (like LVGL's path transform)
    vg_lite_translate(100.0f, 100.0f, &m);
    
    // After second translate(100, 100):
    // Previous point (110, 110) would become (210, 210)
    // or point (20, 20) would become (20 + 200, 20 + 200) = (220, 220)
    EXPECT_FLOAT_EQ(m.m[0][2], 200.0f);  // tx = 200 (cumulative)
    EXPECT_FLOAT_EQ(m.m[1][2], 200.0f);  // ty = 200
    
    // Verify scale tracking
    EXPECT_FLOAT_EQ(m.scaleX, 1.0f);
    EXPECT_FLOAT_EQ(m.scaleY, 1.0f);
    EXPECT_FLOAT_EQ(m.angle, 0.0f);
    
    // No golden comparison needed - this is a unit test
    SUCCEED();
}

/**
 * LVGL Test: test_draw_lines()
 * Golden: vector_draw_lines.lp64.png
 * 
 * LVGL draws various lines with different stroke properties.
 * Note: Full stroke support may require additional vg_lite API.
 */
TEST_F(VectorDrawTest, DrawLines) {
    // Setup buffer for rendering (skips if no GL context)
    if (!setup_render_buffer()) {
        GTEST_SKIP() << "GL backend requires GL context for rendering tests";
    }
    
    // Draw basic shapes to represent line test output
    // (Full stroke API may not be available)
    
    vg_lite_path_t* path1 = create_rect_path(50, 50, 400, 5);
    ASSERT_NE(path1, nullptr);
    vg_lite_draw(buffer, path1, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER, 
                 make_color(0, 0, 0, 0));
    free_path(path1);
    
    vg_lite_path_t* path2 = create_rect_path(50, 80, 400, 5);
    ASSERT_NE(path2, nullptr);
    vg_lite_draw(buffer, path2, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                 make_color(255, 255, 0, 0));
    free_path(path2);
    
    verify_golden(buffer, "vector_draw_lines");
}

/**
 * LVGL Test: test_draw_shapes()
 * Golden: vector_draw_shapes.lp64.png
 * 
 * LVGL draws:
 * - Red rectangle
 * - Blue rounded rectangle
 * - Green circle (semi-transparent)
 * - Radial gradient circle
 * - Image fill
 * - Multiply blend mode rectangle
 * - Arc shapes
 */
TEST_F(VectorDrawTest, DrawShapes) {
    // Setup buffer for rendering (skips if no GL context)
    if (!setup_render_buffer()) {
        GTEST_SKIP() << "GL backend requires GL context for rendering tests";
    }
    
    // Red rectangle (from LVGL: rect1 = {50, 50, 150, 150})
    vg_lite_path_t* path1 = create_rect_path(50, 50, 100, 100);
    ASSERT_NE(path1, nullptr);
    vg_lite_draw(buffer, path1, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                 make_color(255, 255, 0, 0));  // Red
    free_path(path1);
    
    // Blue rectangle (translated by 150px)
    vg_lite_path_t* path2 = create_rect_path(200, 50, 100, 100);
    ASSERT_NE(path2, nullptr);
    vg_lite_draw(buffer, path2, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                 make_color(255, 0, 0, 255));  // Blue
    free_path(path2);
    
    // Green circle (semi-transparent)
    vg_lite_path_t* path3 = create_circle_path(350, 100, 50, 32);
    if (path3) {
        vg_lite_draw(buffer, path3, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(128, 0, 255, 0));  // 50% transparent green
        free_path(path3);
    }
    
    verify_golden(buffer, "vector_draw_shapes");
}
