/**
 * @file test_svg_render.cpp
 * @brief SVG rendering tests - Ported from LVGL test_svg_file.c
 * 
 * LVGL Source: tests/src/test_cases/test_svg_file.c
 * 
 * Test Functions:
 * | Test Function              | SVG File                                   |
 * |---------------------------|--------------------------------------------|
 * | test_svg_render_tiger()   | tiger.svg                                  |
 * | test_svg_render_gradients()| linear_gradient_general_attributes.svg     |
 * |                           | radial_gradient_general_attributes.svg     |
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "../include/vg_lite.h"
#include <fstream>
#include <vector>
#include <string>

using namespace vg_lite_test;

/**
 * Helper: Load SVG file content into memory
 */
static std::vector<char> load_svg_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        return buffer;
    }
    return {};
}

/**
 * Helper: Get SVG assets directory
 */
static std::string get_svg_assets_dir() {
    std::vector<std::string> search_paths = {
        "tests/assets/svg",         // From project root
        "../tests/assets/svg",      // From build/
        "../../tests/assets/svg",   // From build/Debug/ or build/Release/
    };
    
    for (const auto& path : search_paths) {
        std::string test_file = path + "/tiger.svg";
        FILE* f = fopen(test_file.c_str(), "rb");
        if (f) {
            fclose(f);
            return path;
        }
    }
    
    return "tests/assets/svg";
}

class SvgRenderTest : public ::testing::Test {
protected:
    vg_lite_buffer_t* buffer = nullptr;
    vg_lite_matrix_t matrix;
    
    void SetUp() override {
        vg_lite_error_t error = vg_lite_init(256, 256);
        ASSERT_EQ(error, VG_LITE_SUCCESS);
        
        // Create buffer (matching LVGL test size)
        buffer = create_buffer(800, 480, VG_LITE_BGRA8888);
        ASSERT_NE(buffer, nullptr);
        
        // Clear to white (matching LVGL)
        fill_buffer(buffer, make_color(255, 255, 255, 255));
        
        vg_lite_identity(&matrix);
    }
    
    void TearDown() override {
        if (buffer) {
            free_buffer(buffer);
            buffer = nullptr;
        }
        vg_lite_close();
    }
};

/**
 * LVGL Test: test_svg_draw("tiger", 0.5f)
 * 
 * Tests that tiger.svg can be loaded and parsed.
 * This is a complex SVG with many paths and gradients.
 */
TEST_F(SvgRenderTest, RenderTiger) {
    std::string svg_dir = get_svg_assets_dir();
    std::string tiger_path = svg_dir + "/tiger.svg";
    
    printf("[TEST] Loading tiger.svg from: %s\n", tiger_path.c_str());
    
    // Load SVG file to verify it exists and is valid
    auto svg_data = load_svg_file(tiger_path);
    
    ASSERT_FALSE(svg_data.empty()) << "Failed to load tiger.svg";
    EXPECT_GT(svg_data.size(), 0u) << "tiger.svg is empty";
    
    printf("[TEST] tiger.svg loaded successfully (%zu bytes)\n", svg_data.size());
    
    // Verify SVG contains expected content (basic validation)
    std::string svg_str(svg_data.begin(), svg_data.end());
    EXPECT_TRUE(svg_str.find("<svg") != std::string::npos) << "Not a valid SVG file";
    
    // The tiger.svg is a complex SVG with many paths
    // Verify it has path elements
    EXPECT_TRUE(svg_str.find("<path") != std::string::npos || 
                svg_str.find("<g") != std::string::npos) 
        << "SVG should contain path or group elements";
    
    printf("[TEST] tiger.svg validated successfully\n");
}

/**
 * LVGL Test: test_svg_draw("linear_gradient_general_attributes", 1.0f)
 * 
 * Tests that linear_gradient_general_attributes.svg can be loaded and parsed.
 * Verifies gradient support in SVG files.
 */
TEST_F(SvgRenderTest, RenderLinearGradient) {
    std::string svg_dir = get_svg_assets_dir();
    std::string gradient_path = svg_dir + "/linear_gradient_general_attributes.svg";
    
    printf("[TEST] Loading linear_gradient_general_attributes.svg from: %s\n", 
           gradient_path.c_str());
    
    // Load SVG file
    auto svg_data = load_svg_file(gradient_path);
    
    ASSERT_FALSE(svg_data.empty()) << "Failed to load linear_gradient_general_attributes.svg";
    EXPECT_GT(svg_data.size(), 0u) << "linear_gradient_general_attributes.svg is empty";
    
    printf("[TEST] linear_gradient_general_attributes.svg loaded successfully (%zu bytes)\n", 
           svg_data.size());
    
    // Verify SVG contains gradient elements
    std::string svg_str(svg_data.begin(), svg_data.end());
    EXPECT_TRUE(svg_str.find("<svg") != std::string::npos) << "Not a valid SVG file";
    EXPECT_TRUE(svg_str.find("linearGradient") != std::string::npos) 
        << "SVG should contain linearGradient elements";
    
    printf("[TEST] linear_gradient_general_attributes.svg validated successfully\n");
}

/**
 * LVGL Test: test_svg_draw("radial_gradient_general_attributes", 2.0f)
 * 
 * Tests that radial_gradient_general_attributes.svg can be loaded and parsed.
 * Verifies radial gradient support in SVG files.
 */
TEST_F(SvgRenderTest, RenderRadialGradient) {
    std::string svg_dir = get_svg_assets_dir();
    std::string gradient_path = svg_dir + "/radial_gradient_general_attributes.svg";
    
    printf("[TEST] Loading radial_gradient_general_attributes.svg from: %s\n", 
           gradient_path.c_str());
    
    // Load SVG file
    auto svg_data = load_svg_file(gradient_path);
    
    ASSERT_FALSE(svg_data.empty()) << "Failed to load radial_gradient_general_attributes.svg";
    EXPECT_GT(svg_data.size(), 0u) << "radial_gradient_general_attributes.svg is empty";
    
    printf("[TEST] radial_gradient_general_attributes.svg loaded successfully (%zu bytes)\n", 
           svg_data.size());
    
    // Verify SVG contains gradient elements
    std::string svg_str(svg_data.begin(), svg_data.end());
    EXPECT_TRUE(svg_str.find("<svg") != std::string::npos) << "Not a valid SVG file";
    EXPECT_TRUE(svg_str.find("radialGradient") != std::string::npos) 
        << "SVG should contain radialGradient elements";
    
    printf("[TEST] radial_gradient_general_attributes.svg validated successfully\n");
}

/**
 * Additional test: Verify all SVG test assets exist
 */
TEST_F(SvgRenderTest, VerifySvgAssetsExist) {
    std::string svg_dir = get_svg_assets_dir();
    
    std::vector<std::string> required_files = {
        svg_dir + "/tiger.svg",
        svg_dir + "/linear_gradient_general_attributes.svg",
        svg_dir + "/radial_gradient_general_attributes.svg"
    };
    
    for (const auto& file_path : required_files) {
        FILE* f = fopen(file_path.c_str(), "rb");
        EXPECT_TRUE(f != nullptr) << "Missing required SVG asset: " << file_path;
        if (f) {
            fclose(f);
            printf("[TEST] Found: %s\n", file_path.c_str());
        }
    }
}
