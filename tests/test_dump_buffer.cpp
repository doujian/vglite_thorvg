/**
 * @file test_dump_buffer.cpp
 * @brief Test for vg_lite_dump_buffer() function
 *
 * Tests:
 * 1. Create buffer -> render -> dump to PNG -> verify file exists
 * 2. Clean up generated PNG files
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "../include/vg_lite.h"
#include <stdio.h>
#include <filesystem>

using namespace vg_lite_test;

/**
 * Dump Buffer Test Fixture
 * Tests buffer dump functionality to PNG files
 */
class DumpBufferTest : public ::testing::Test {
protected:
    vg_lite_buffer_t* buffer = nullptr;
    std::string temp_filename;

    void SetUp() override {
        // Initialize VGLite
        vg_lite_error_t error = vg_lite_init(256, 256);
        ASSERT_EQ(error, VG_LITE_SUCCESS);
    }

    void TearDown() override {
        // Clean up buffer
        if (buffer) {
            free_buffer(buffer);
            buffer = nullptr;
        }
        
        // Clean up temp PNG file
        if (!temp_filename.empty()) {
            std::remove(temp_filename.c_str());
        }
        
        vg_lite_close();
    }

    /**
     * Helper to verify file exists
     */
    bool file_exists(const std::string& path) {
        FILE* f = fopen(path.c_str(), "rb");
        if (f) {
            fclose(f);
            return true;
        }
        return false;
    }
};

/**
 * Test: Basic dump to PNG file
 * Verifies that vg_lite_dump_buffer() creates a valid PNG file
 */
TEST_F(DumpBufferTest, BasicDumpToPng) {
    // 1. Create a buffer
    buffer = create_buffer(100, 100, VG_LITE_BGRA8888);
    ASSERT_NE(buffer, nullptr);
    ASSERT_NE(buffer->memory, nullptr);

    // 2. Fill buffer with a color pattern
    vg_lite_color_t red = make_color(255, 255, 0, 0);
    fill_buffer(buffer, red);

    // 3. Set up temp file path (use system temp directory)
    temp_filename = "test_dump_buffer_basic.png";

    // 4. Call vg_lite_dump_buffer()
    vg_lite_error_t error = vg_lite_dump_buffer(buffer, temp_filename.c_str());
    EXPECT_EQ(error, VG_LITE_SUCCESS);

    // 5. Verify file exists
    EXPECT_TRUE(file_exists(temp_filename)) << "PNG file was not created: " << temp_filename;
}

/**
 * Test: Dump with different sizes
 * Verifies dump works with various buffer dimensions
 */
TEST_F(DumpBufferTest, DifferentSizes) {
    struct SizeTest {
        uint32_t width;
        uint32_t height;
    };

    SizeTest sizes[] = {
        {16, 16},
        {64, 64},
        {100, 50},
        {50, 100},
    };

    for (const auto& size : sizes) {
        SCOPED_TRACE("Testing size " + std::to_string(size.width) + "x" + std::to_string(size.height));
        
        // Create buffer
        if (buffer) free_buffer(buffer);
        buffer = create_buffer(size.width, size.height, VG_LITE_BGRA8888);
        ASSERT_NE(buffer, nullptr);

        // Fill with unique color based on size
        vg_lite_color_t color = make_color(128, (uint8_t)size.width, (uint8_t)size.height, 64);
        fill_buffer(buffer, color);

        // Generate unique filename
        char filename[64];
        sprintf(filename, "test_dump_size_%ux%u.png", size.width, size.height);
        temp_filename = filename;

        // Dump to PNG
        vg_lite_error_t error = vg_lite_dump_buffer(buffer, temp_filename.c_str());
        EXPECT_EQ(error, VG_LITE_SUCCESS) << "Failed for size " << size.width << "x" << size.height;
        EXPECT_TRUE(file_exists(temp_filename)) << "PNG not created for size " << size.width << "x" << size.height;
    }
}

/**
 * Test: Dump with RGB565 format
 * Verifies dump works with RGB565 format
 */
TEST_F(DumpBufferTest, Rgb565Format) {
    // Create buffer with RGB565 format
    buffer = create_buffer(64, 64, VG_LITE_RGB565);
    ASSERT_NE(buffer, nullptr);

    // Fill with color
    vg_lite_color_t green = make_color(255, 0, 255, 0);
    fill_buffer(buffer, green);

    // Set temp filename
    temp_filename = "test_dump_rgb565.png";

    // Dump to PNG
    vg_lite_error_t error = vg_lite_dump_buffer(buffer, temp_filename.c_str());
    EXPECT_EQ(error, VG_LITE_SUCCESS);

    // Verify file exists
    EXPECT_TRUE(file_exists(temp_filename));
}

/**
 * Test: Dump with L8 format
 * Verifies dump works with L8 (luminance) format
 */
TEST_F(DumpBufferTest, L8Format) {
    // Create buffer with L8 format
    buffer = create_buffer(64, 64, VG_LITE_L8);
    ASSERT_NE(buffer, nullptr);

    // Fill with luminance value (using BGRA8888 format for simplicity)
    vg_lite_color_t gray = make_color(255, 128, 128, 128);  // Gray in BGRA
    fill_buffer(buffer, gray);

    // Set temp filename
    temp_filename = "test_dump_l8.png";

    // Dump to PNG
    vg_lite_error_t error = vg_lite_dump_buffer(buffer, temp_filename.c_str());
    EXPECT_EQ(error, VG_LITE_SUCCESS);

    // Verify file exists
    EXPECT_TRUE(file_exists(temp_filename));
}

/**
 * Test: Null buffer returns error
 * Verifies proper error handling for null buffer
 */
TEST_F(DumpBufferTest, NullBufferError) {
    temp_filename = "test_dump_null_buffer.png";
    
    vg_lite_error_t error = vg_lite_dump_buffer(nullptr, temp_filename.c_str());
    EXPECT_EQ(error, VG_LITE_INVALID_ARGUMENT);
    
    // File should NOT be created
    EXPECT_FALSE(file_exists(temp_filename));
}

/**
 * Test: Null filename returns error
 * Verifies proper error handling for null filename
 */
TEST_F(DumpBufferTest, NullFilenameError) {
    buffer = create_buffer(32, 32, VG_LITE_BGRA8888);
    ASSERT_NE(buffer, nullptr);
    
    vg_lite_error_t error = vg_lite_dump_buffer(buffer, nullptr);
    EXPECT_EQ(error, VG_LITE_INVALID_ARGUMENT);
}

/**
 * Test: Multiple dumps don't interfere
 * Verifies multiple dump calls work correctly
 */
TEST_F(DumpBufferTest, MultipleDumps) {
    // Create buffer
    buffer = create_buffer(32, 32, VG_LITE_BGRA8888);
    ASSERT_NE(buffer, nullptr);

    // First dump
    vg_lite_color_t red = make_color(255, 255, 0, 0);
    fill_buffer(buffer, red);
    
    std::string filename1 = "test_dump_multi_1.png";
    vg_lite_error_t error = vg_lite_dump_buffer(buffer, filename1.c_str());
    EXPECT_EQ(error, VG_LITE_SUCCESS);
    EXPECT_TRUE(file_exists(filename1));

    // Clear buffer and second dump
    vg_lite_color_t blue = make_color(255, 0, 0, 255);
    fill_buffer(buffer, blue);
    
    std::string filename2 = "test_dump_multi_2.png";
    error = vg_lite_dump_buffer(buffer, filename2.c_str());
    EXPECT_EQ(error, VG_LITE_SUCCESS);
    EXPECT_TRUE(file_exists(filename2));

    // Clean up second file manually
    std::remove(filename1.c_str());
    std::remove(filename2.c_str());
}