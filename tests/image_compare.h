/**
 * @file image_compare.h
 * @brief Golden image comparison utilities for VGLite tests
 * 
 * Uses stb_image for PNG reading/writing
 * Note: STB implementations are in stb_impl.cpp
 */

#ifndef IMAGE_COMPARE_H
#define IMAGE_COMPARE_H

#include <stdint.h>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>

// STB implementations are in stb_impl.cpp - only declare here
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

namespace vg_lite_test {

/**
 * Simple RGBA image container
 */
struct Image {
    int width;
    int height;
    std::vector<uint8_t> data;  // RGBA format
    
    Image() : width(0), height(0) {}
    
    Image(int w, int h) : width(w), height(h), data(w * h * 4, 0) {}
    
    bool valid() const { return width > 0 && height > 0 && !data.empty(); }
    
    size_t size() const { return data.size(); }
    
    uint8_t* ptr() { return data.data(); }
    const uint8_t* ptr() const { return data.data(); }
    
    // Get pixel at (x, y) as BGRA (vg_lite format)
    uint32_t getPixelBGRA(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return 0;
        const uint8_t* p = &data[(y * width + x) * 4];
        // Convert from RGBA to BGRA
        return ((uint32_t)p[3] << 24) | ((uint32_t)p[0] << 16) | 
               ((uint32_t)p[1] << 8) | (uint32_t)p[2];
    }
    
    // Set pixel at (x, y) from BGRA (vg_lite format)
    void setPixelBGRA(int x, int y, uint32_t bgra) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        uint8_t* p = &data[(y * width + x) * 4];
        // Convert from BGRA to RGBA
        p[0] = (bgra >> 16) & 0xFF;  // R
        p[1] = (bgra >> 8) & 0xFF;   // G
        p[2] = bgra & 0xFF;          // B
        p[3] = (bgra >> 24) & 0xFF;  // A
    }
};

/**
 * Load PNG image from file
 */
inline Image loadImage(const std::string& path) {
    Image img;
    int channels;
    uint8_t* data = stbi_load(path.c_str(), &img.width, &img.height, &channels, 4);
    if (data) {
        img.data.assign(data, data + img.width * img.height * 4);
        stbi_image_free(data);
    }
    return img;
}

/**
 * Save image to PNG file
 */
inline bool saveImage(const std::string& path, const Image& img) {
    if (!img.valid()) return false;
    return stbi_write_png(path.c_str(), img.width, img.height, 4, 
                          img.data.data(), img.width * 4) != 0;
}

/**
 * Save vg_lite_buffer_t to PNG file
 */
inline bool saveBufferToPng(const std::string& path, const uint8_t* data, 
                            int width, int height, int format) {
    Image img(width, height);
    
    // Convert from vg_lite format to RGBA
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint32_t pixel;
            const uint8_t* src = data + (y * width + x) * 4;
            
            // Assuming BGRA8888 format (most common)
            pixel = ((uint32_t)src[3] << 24) | ((uint32_t)src[2] << 16) |
                    ((uint32_t)src[1] << 8) | (uint32_t)src[0];
            
            img.setPixelBGRA(x, y, pixel);
        }
    }
    
    return saveImage(path, img);
}

/**
 * Compare result with tolerance
 */
struct CompareResult {
    bool match;
    double difference_percent;
    int mismatched_pixels;
    std::string message;
    
    CompareResult() : match(false), difference_percent(100.0), mismatched_pixels(0) {}
};

inline CompareResult compareImages(const Image& actual, const Image& expected, 
                                    double tolerance = 0.01) {
    CompareResult result;
    
    if (!actual.valid() || !expected.valid()) {
        result.message = "Invalid image(s)";
        return result;
    }
    
    if (actual.width != expected.width || actual.height != expected.height) {
        result.message = "Size mismatch: actual(" + 
                         std::to_string(actual.width) + "x" + std::to_string(actual.height) +
                         ") vs expected(" + 
                         std::to_string(expected.width) + "x" + std::to_string(expected.height) + ")";
        return result;
    }
    
    int totalPixels = actual.width * actual.height;
    int maxDiff = static_cast<int>(255 * tolerance);  // Per-channel tolerance
    result.mismatched_pixels = 0;
    double totalDiff = 0;
    
    for (size_t i = 0; i < actual.data.size(); i += 4) {
        int dr = std::abs((int)actual.data[i] - (int)expected.data[i]);
        int dg = std::abs((int)actual.data[i+1] - (int)expected.data[i+1]);
        int db = std::abs((int)actual.data[i+2] - (int)expected.data[i+2]);
        int da = std::abs((int)actual.data[i+3] - (int)expected.data[i+3]);
        
        int maxChannelDiff = std::max({dr, dg, db, da});
        totalDiff += maxChannelDiff;
        
        if (maxChannelDiff > maxDiff) {
            result.mismatched_pixels++;
        }
    }
    
    result.difference_percent = (totalDiff / (totalPixels * 4 * 255.0)) * 100.0;
    result.match = (result.mismatched_pixels == 0) || 
                   (result.difference_percent <= tolerance * 100.0);
    
    if (!result.match) {
        result.message = "Image mismatch: " + std::to_string(result.mismatched_pixels) +
                         "/" + std::to_string(totalPixels) + " pixels differ (" +
                         std::to_string(result.difference_percent) + "% difference)";
    } else {
        result.message = "Images match within tolerance";
    }
    
    return result;
}

/**
 * Convert vg_lite_buffer_t to Image
 */
inline Image bufferToImage(const uint8_t* data, int width, int height) {
    Image img(width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            size_t offset = (y * width + x) * 4;
            // Assume BGRA8888 format
            uint32_t bgra = ((uint32_t)data[offset + 3] << 24) |
                           ((uint32_t)data[offset + 2] << 16) |
                           ((uint32_t)data[offset + 1] << 8) |
                           (uint32_t)data[offset];
            img.setPixelBGRA(x, y, bgra);
        }
    }
    
    return img;
}

} // namespace vg_lite_test

#endif // IMAGE_COMPARE_H
