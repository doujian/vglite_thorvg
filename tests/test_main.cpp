/**
 * @file test_main.cpp
 * @brief Main test runner for VGLite Render using Google Test
 */

#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include "vg_lite.h"
#include "test_helpers.h"

using namespace vg_lite_test;

// Global variables for dump framework
bool g_dump_enabled = false;
std::string g_output_dir = ".";

int main(int argc, char** argv) {
    // Parse CLI arguments before InitGoogleTest
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--dump") {
            g_dump_enabled = true;
        } else if (arg.substr(0, 14) == "--output-dir=") {
            g_output_dir = arg.substr(14);
        }
    }

    std::cout << "Test configuration:" << std::endl;
    std::cout << "  dump_enabled: " << (g_dump_enabled ? "true" : "false") << std::endl;
    std::cout << "  output_dir: " << g_output_dir << std::endl;

    // Initialize GL context for GL backend (backend 2) before running tests
    // GLContextManager singleton will be destroyed automatically at program exit
#if VG_LITE_RENDER_BACKEND == 2
    if (!GLContextManager::instance().initialize()) {
        std::cerr << "Failed to initialize GL context for GL backend tests!" << std::endl;
        return 1;
    }
    std::cout << "  GL context initialized for GL backend" << std::endl;
#endif

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/* Test basic init/close */
TEST_F(VGLiteTest, InitClose) {
    vg_lite_error_t error;
    
    error = vg_lite_init(256, 256);
    EXPECT_EQ(VG_LITE_SUCCESS, error);
    
    error = vg_lite_close();
    EXPECT_EQ(VG_LITE_SUCCESS, error);
}

/* Test matrix identity */
TEST_F(VGLiteTest, MatrixIdentity) {
    vg_lite_matrix_t matrix;
    vg_lite_error_t error;
    
    error = vg_lite_identity(&matrix);
    EXPECT_EQ(VG_LITE_SUCCESS, error);
    
    /* Check identity matrix values */
    EXPECT_FLOAT_EQ(1.0f, matrix.m[0][0]);
    EXPECT_FLOAT_EQ(0.0f, matrix.m[0][1]);
    EXPECT_FLOAT_EQ(0.0f, matrix.m[0][2]);
    EXPECT_FLOAT_EQ(0.0f, matrix.m[1][0]);
    EXPECT_FLOAT_EQ(1.0f, matrix.m[1][1]);
    EXPECT_FLOAT_EQ(0.0f, matrix.m[1][2]);
    EXPECT_FLOAT_EQ(0.0f, matrix.m[2][0]);
    EXPECT_FLOAT_EQ(0.0f, matrix.m[2][1]);
    EXPECT_FLOAT_EQ(1.0f, matrix.m[2][2]);
}

/* Test matrix translate */
TEST_F(VGLiteTest, MatrixTranslate) {
    vg_lite_matrix_t matrix;
    vg_lite_error_t error;
    
    error = vg_lite_identity(&matrix);
    EXPECT_EQ(VG_LITE_SUCCESS, error);
    
    error = vg_lite_translate(100.0f, 50.0f, &matrix);
    EXPECT_EQ(VG_LITE_SUCCESS, error);
    
    /* Check translation values */
    EXPECT_FLOAT_EQ(100.0f, matrix.m[0][2]);
    EXPECT_FLOAT_EQ(50.0f, matrix.m[1][2]);
}

/* Test matrix scale */
TEST_F(VGLiteTest, MatrixScale) {
    vg_lite_matrix_t matrix;
    vg_lite_error_t error;
    
    error = vg_lite_identity(&matrix);
    EXPECT_EQ(VG_LITE_SUCCESS, error);
    
    error = vg_lite_scale(2.0f, 3.0f, &matrix);
    EXPECT_EQ(VG_LITE_SUCCESS, error);
    
    /* Check scale values */
    EXPECT_FLOAT_EQ(2.0f, matrix.m[0][0]);
    EXPECT_FLOAT_EQ(3.0f, matrix.m[1][1]);
}

/* Test matrix rotate */
TEST_F(VGLiteTest, MatrixRotate) {
    vg_lite_matrix_t matrix;
    vg_lite_error_t error;
    
    error = vg_lite_identity(&matrix);
    EXPECT_EQ(VG_LITE_SUCCESS, error);
    
    error = vg_lite_rotate(90.0f, &matrix);
    EXPECT_EQ(VG_LITE_SUCCESS, error);
    
    /* Check rotation values (90 degrees) */
    EXPECT_NEAR(0.0f, matrix.m[0][0], 0.0001f);
    EXPECT_NEAR(-1.0f, matrix.m[0][1], 0.0001f);
    EXPECT_NEAR(1.0f, matrix.m[1][0], 0.0001f);
    EXPECT_NEAR(0.0f, matrix.m[1][1], 0.0001f);
}
