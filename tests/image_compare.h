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

// Include vg_lite for format constants
#include "../include/vg_lite.h"

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
    if (!img.valid()) {
        fprintf(stderr, "[ERROR] saveImage: Invalid image (w=%d, h=%d, size=%zu)\n", 
                img.width, img.height, img.data.size());
        return false;
    }
    
    // Ensure directory exists (basic check)
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        // Path contains directory - note: we don't create directories here
    }
    
    int result = stbi_write_png(path.c_str(), img.width, img.height, 4, 
                                 img.data.data(), img.width * 4);
    if (result == 0) {
        fprintf(stderr, "[ERROR] saveImage: stbi_write_png failed for path: %s\n", path.c_str());
        return false;
    }
    
    // Verify file was created
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) {
        fprintf(stderr, "[ERROR] saveImage: File was not created: %s\n", path.c_str());
        return false;
    }
    fclose(f);
    
    return true;
}

// Forward declaration - defined below
inline Image bufferToImage(const uint8_t* data, int width, int height, 
                           int format, int stride);

/**
 * Save vg_lite_buffer_t to PNG file
 * Supports multiple formats: BGRA8888, ARGB1555, RGB565, etc.
 */
inline bool saveBufferToPng(const std::string& path, const uint8_t* data, 
                            int width, int height, int format, int stride = 0) {
    // Validate input
    if (!data || width <= 0 || height <= 0) {
        fprintf(stderr, "[ERROR] saveBufferToPng: Invalid parameters (data=%p, w=%d, h=%d)\n", 
                (void*)data, width, height);
        return false;
    }
    
    // Reuse bufferToImage for format conversion
    Image img = bufferToImage(data, width, height, format, stride);
    if (!img.valid()) {
        fprintf(stderr, "[ERROR] saveBufferToPng: Failed to convert buffer to image\n");
        return false;
    }
    
    bool result = saveImage(path, img);
    if (!result) {
        fprintf(stderr, "[ERROR] saveBufferToPng: stbi_write_png failed for path: %s\n", path.c_str());
    }
    return result;
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
 * Supports multiple formats: BGRA8888, ARGB1555, RGB565, etc.
 */
inline Image bufferToImage(const uint8_t* data, int width, int height, 
                           int format = VG_LITE_BGRA8888, int stride = 0) {
    Image img(width, height);
    
    // Default stride to width * bytes_per_pixel
    if (stride == 0) {
        switch (format) {
            case VG_LITE_ARGB1555:
            case VG_LITE_RGB565:
            case VG_LITE_ARGB4444:
            case VG_LITE_BGRA5551:
            case VG_LITE_BGRA4444:
            case VG_LITE_BGR565:
                stride = width * 2;
                break;
            case VG_LITE_L8:
            case VG_LITE_A8:
                stride = width;
                break;
            default:
                stride = width * 4;
                break;
        }
    }
    
    switch (format) {
        case VG_LITE_ARGB1555: {
            // ARGB1555: A(1) R(5) G(5) B(5) = 16 bits
            const uint16_t* src = (const uint16_t*)data;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    uint16_t pixel = src[y * (stride / 2) + x];
                    uint8_t a = (pixel >> 15) & 0x01;
                    uint8_t r = (pixel >> 10) & 0x1F;
                    uint8_t g = (pixel >> 5) & 0x1F;
                    uint8_t b = pixel & 0x1F;
                    // Convert 5-bit to 8-bit
                    r = (r * 255) / 31;
                    g = (g * 255) / 31;
                    b = (b * 255) / 31;
                    a = a ? 255 : 0;
                    img.setPixelBGRA(x, y, (a << 24) | (r << 16) | (g << 8) | b);
                }
            }
            break;
        }
        
        case VG_LITE_RGB565:
        case VG_LITE_BGR565: {
            // RGB565: R(5) G(6) B(5) = 16 bits
            const uint16_t* src = (const uint16_t*)data;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    uint16_t pixel = src[y * (stride / 2) + x];
                    uint8_t r, g, b;
                    if (format == VG_LITE_RGB565) {
                        r = (pixel >> 11) & 0x1F;
                        g = (pixel >> 5) & 0x3F;
                        b = pixel & 0x1F;
                    } else {
                        b = (pixel >> 11) & 0x1F;
                        g = (pixel >> 5) & 0x3F;
                        r = pixel & 0x1F;
                    }
                    r = (r * 255) / 31;
                    g = (g * 255) / 63;
                    b = (b * 255) / 31;
                    img.setPixelBGRA(x, y, (0xFF << 24) | (r << 16) | (g << 8) | b);
                }
            }
            break;
        }
        
        case VG_LITE_ARGB4444:
        case VG_LITE_BGRA4444: {
            // ARGB4444: A(4) R(4) G(4) B(4) = 16 bits
            const uint16_t* src = (const uint16_t*)data;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    uint16_t pixel = src[y * (stride / 2) + x];
                    uint8_t a, r, g, b;
                    if (format == VG_LITE_ARGB4444) {
                        a = (pixel >> 12) & 0x0F;
                        r = (pixel >> 8) & 0x0F;
                        g = (pixel >> 4) & 0x0F;
                        b = pixel & 0x0F;
                    } else {
                        a = (pixel >> 12) & 0x0F;
                        b = (pixel >> 8) & 0x0F;
                        g = (pixel >> 4) & 0x0F;
                        r = pixel & 0x0F;
                    }
                    // Convert 4-bit to 8-bit
                    a = a | (a << 4);
                    r = r | (r << 4);
                    g = g | (g << 4);
                    b = b | (b << 4);
                    img.setPixelBGRA(x, y, (a << 24) | (r << 16) | (g << 8) | b);
                }
            }
            break;
        }
        
        case VG_LITE_L8: {
            // L8: 8-bit luminance
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    uint8_t l = data[y * stride + x];
                    img.setPixelBGRA(x, y, (0xFF << 24) | (l << 16) | (l << 8) | l);
                }
            }
            break;
        }
        
        case VG_LITE_A8: {
            // A8: 8-bit alpha
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    uint8_t a = data[y * stride + x];
                    img.setPixelBGRA(x, y, (a << 24) | (0xFF << 16) | (0xFF << 8) | 0xFF);
                }
            }
            break;
        }
        
        default: {
            // BGRA8888 or other 32-bit formats
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    size_t offset = y * stride + x * 4;
                    uint32_t bgra = ((uint32_t)data[offset + 3] << 24) |
                                   ((uint32_t)data[offset + 2] << 16) |
                                   ((uint32_t)data[offset + 1] << 8) |
                                   (uint32_t)data[offset];
                    img.setPixelBGRA(x, y, bgra);
                }
            }
            break;
        }
    }
    
    return img;
}

} // namespace vg_lite_test

#endif // IMAGE_COMPARE_H
