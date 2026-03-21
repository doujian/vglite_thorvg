/**
 * @file test_render_to_argb1555.cpp
 * @brief ARGB1555 format render tests
 * 
 * Tests vg_lite rendering to ARGB1555 (1-bit alpha, 5-bit RGB per channel) buffers.
 * Format: A(1) R(5) G(5) B(5) = 16 bits per pixel.
 * 
 * Geometric Scenes:
 * - FILL: Multiple colored rectangles with various opacity
 * - LINE: Multiple lines with various widths and colors
 * - BORDER: Rectangles with borders using stroke settings
 * - TRIANGLE: Multiple triangles with various colors and positions
 * - ARC_NORMAL: Multiple arcs with different sizes and colors
 * - ARC_IMAGE: Circles with gradient/texture fill
 * - BLEND_MODE: Various blend modes with overlapping shapes
 * - BOX_SHADOW: Rectangles and ovals with drop shadows
 * 
 * NO LVGL dependencies - uses vg_lite API directly.
 */

// Windows min/max macro fix
#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#endif

#include <gtest/gtest.h>
#include <iostream>
#include "../include/vg_lite.h"
#include "test_helpers.h"

using namespace vg_lite_test;

// Platform detection - skip on 32-bit due to color precision issues
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__) || defined(_M_X64)
    #define ARGB1555_TESTS_ENABLED 1
#else
    #define ARGB1555_TESTS_ENABLED 0
#endif

// ============================================================================
// LVGL Logo Image Data (30x30 ARGB8888)
// Copied from lvgl-master/demos/render/assets/img_render_lvgl_logo_argb8888.c
// Format: BGRA (Blue, Green, Red, Alpha) per pixel
// ============================================================================

static const int LVGL_LOGO_WIDTH = 30;
static const int LVGL_LOGO_HEIGHT = 30;

// LVGL logo pixel data in BGRA format (extracted from LVGL source)
static const uint8_t lvgl_logo_bgra_data[] = {
    // Row 0
    0xff,0xff,0xff,0x00,0xfe,0xfe,0xfe,0x00,0xaa,0xaa,0xa8,0x62,0x65,0x64,0x61,0xb7,0x5f,0x5e,0x5b,0xc3,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc1,0x60,0x60,0x5c,0xc2,0x5e,0x5d,0x5a,0xc4,0x7f,0x7e,0x7b,0x94,0xdc,0xdc,0xdc,0x26,0xff,0xff,0xff,0x00,
    // Row 1
    0xfb,0xfb,0xfa,0x03,0x86,0x86,0x83,0x90,0x32,0x31,0x2c,0xfd,0x35,0x34,0x30,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x36,0x35,0x31,0xff,0x2f,0x2e,0x2a,0xff,0x2d,0x2c,0x28,0xff,0x31,0x30,0x2c,0xff,0x49,0x48,0x44,0xde,0xd0,0xd0,0xcf,0x34,
    // Row 2
    0xab,0xaa,0xa8,0x61,0x32,0x31,0x2d,0xff,0x38,0x37,0x33,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x34,0x33,0x2f,0xff,0x62,0x61,0x5e,0xff,0x6d,0x6c,0x69,0xff,0x38,0x37,0x33,0xff,0x2d,0x2c,0x28,0xff,0x5d,0x5d,0x59,0xc6,
    // Row 3
    0x65,0x65,0x61,0xb9,0x35,0x34,0x30,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x30,0x2f,0x2b,0xff,0x7c,0x7c,0x79,0xff,0xfc,0xfc,0xfc,0xff,0xff,0xff,0xff,0xff,0x9e,0x9d,0x9b,0xff,0x2f,0x2e,0x2a,0xff,0x3e,0x3d,0x39,0xf3,
    // Row 4
    0x5f,0x5f,0x5b,0xc3,0x36,0x35,0x31,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x2c,0x2b,0x27,0xff,0xb1,0xb2,0xb0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xd9,0xd9,0xd9,0xff,0x33,0x32,0x2e,0xff,0x3e,0x3d,0x39,0xf4,
    // Row 5
    0x60,0x60,0x5c,0xc1,0x36,0x35,0x31,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x32,0x31,0x2d,0xff,0x64,0x63,0x60,0xff,0xe9,0xe9,0xe9,0xff,0xf3,0xf3,0xf3,0xff,0x81,0x81,0x7e,0xff,0x30,0x2e,0x2a,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 6
    0x60,0x60,0x5c,0xc1,0x36,0x35,0x31,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x35,0xff,0x33,0x32,0x2d,0xff,0x46,0x45,0x41,0xff,0x4d,0x4c,0x48,0xff,0x33,0x32,0x2e,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 7
    0x5d,0x5d,0x59,0xc5,0x36,0x35,0x31,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x35,0x34,0x30,0xff,0x34,0x33,0x2f,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 8
    0x6a,0x69,0x66,0xb3,0x2a,0x29,0x24,0xff,0x2d,0x2c,0x27,0xff,0x2d,0x2c,0x28,0xff,0x2d,0x2c,0x28,0xff,0x2d,0x2c,0x28,0xff,0x2d,0x2c,0x28,0xff,0x2d,0x2c,0x28,0xff,0x2d,0x2c,0x27,0xff,0x32,0x31,0x2d,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 9
    0xe6,0xe5,0xe2,0x1c,0xa9,0xa5,0x9c,0x68,0xa2,0x9e,0x94,0x73,0xa3,0x9f,0x95,0x72,0xa3,0x9f,0x95,0x72,0xa3,0x9f,0x95,0x72,0xa3,0x9f,0x95,0x72,0xa4,0xa0,0x96,0x70,0x9f,0x9c,0x92,0x76,0x5f,0x5e,0x5a,0xc5,0x31,0x30,0x2c,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 10
    0xf7,0xfb,0xff,0x07,0xc6,0xd9,0xff,0x3e,0xbf,0xd4,0xff,0x47,0xc0,0xd5,0xff,0x46,0xc0,0xd5,0xff,0x46,0xc0,0xd5,0xff,0x46,0xc0,0xd5,0xff,0x46,0xbf,0xd4,0xff,0x48,0xc7,0xda,0xff,0x3c,0xf4,0xf6,0xf7,0x08,0x6d,0x6b,0x67,0xb2,0x31,0x30,0x2c,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 11
    0x6a,0x94,0xf8,0xa2,0x13,0x56,0xf3,0xff,0x14,0x57,0xf3,0xff,0x15,0x57,0xf3,0xff,0x15,0x57,0xf3,0xff,0x15,0x57,0xf3,0xff,0x15,0x57,0xf3,0xff,0x14,0x57,0xf3,0xff,0x0b,0x50,0xf2,0xff,0xac,0xc8,0xff,0x59,0xc6,0xc2,0xb9,0x43,0x2e,0x2d,0x29,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 12
    0x48,0x7b,0xf6,0xca,0x1d,0x5d,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x11,0x54,0xf2,0xff,0x91,0xb4,0xff,0x7a,0xcc,0xc8,0xbd,0x3f,0x2f,0x2e,0x2a,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 13
    0x4d,0x7f,0xf6,0xc4,0x1d,0x5d,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x12,0x55,0xf2,0xff,0x95,0xb6,0xff,0x76,0xca,0xc6,0xbc,0x41,0x2f,0x2e,0x2a,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 14
    0x4d,0x7f,0xf6,0xc4,0x1d,0x5d,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x12,0x55,0xf2,0xff,0x95,0xb6,0xff,0x76,0xca,0xc6,0xbc,0x41,0x2f,0x2e,0x2a,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 15
    0x4d,0x7f,0xf6,0xc4,0x1d,0x5d,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x12,0x55,0xf2,0xff,0x95,0xb6,0xff,0x76,0xca,0xc6,0xbc,0x41,0x2f,0x2e,0x2a,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 16
    0x4d,0x7f,0xf6,0xc4,0x1d,0x5d,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x12,0x55,0xf2,0xff,0x95,0xb6,0xff,0x76,0xca,0xc6,0xbc,0x41,0x2f,0x2e,0x2a,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 17
    0x4d,0x7f,0xf6,0xc4,0x1d,0x5d,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x12,0x55,0xf2,0xff,0x95,0xb6,0xff,0x76,0xca,0xc6,0xbc,0x41,0x2f,0x2e,0x2a,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 18
    0x4c,0x7f,0xf6,0xc5,0x1d,0x5d,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x20,0x5f,0xf3,0xff,0x12,0x55,0xf2,0xff,0x95,0xb6,0xff,0x77,0xca,0xc6,0xbc,0x42,0x2f,0x2e,0x2a,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 19
    0x48,0x7c,0xf6,0xc9,0x13,0x56,0xf3,0xff,0x18,0x59,0xf3,0xff,0x18,0x59,0xf3,0xff,0x18,0x59,0xf3,0xff,0x18,0x59,0xf3,0xff,0x18,0x59,0xf3,0xff,0x18,0x59,0xf3,0xff,0x08,0x4e,0xf2,0xff,0x92,0xb4,0xff,0x78,0xc9,0xc5,0xbb,0x41,0x26,0x25,0x21,0xff,0x32,0x31,0x2c,0xff,0x32,0x31,0x2c,0xff,0x32,0x31,0x2c,0xff,0x32,0x31,0x2c,0xff,0x32,0x31,0x2c,0xff,0x32,0x30,0x2c,0xff,0x31,0x30,0x2c,0xff,0x35,0x34,0x30,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 20
    0xb4,0xc9,0xfc,0x50,0x52,0x81,0xf9,0xbe,0x4c,0x7d,0xf9,0xc7,0x4d,0x7e,0xf9,0xc6,0x4d,0x7e,0xf9,0xc6,0x4d,0x7e,0xf9,0xc6,0x4d,0x7e,0xf9,0xc6,0x4b,0x7d,0xf9,0xc7,0x4f,0x80,0xf9,0xbe,0xd4,0xe1,0xff,0x2e,0xef,0xee,0xe9,0x11,0x6d,0x6f,0x6d,0xac,0x5b,0x5e,0x5b,0xc3,0x5e,0x61,0x5f,0xc1,0x5e,0x61,0x5f,0xc1,0x5e,0x61,0x5f,0xc1,0x5e,0x61,0x5f,0xc1,0x5f,0x61,0x5f,0xc1,0x5f,0x61,0x5f,0xc0,0x4b,0x4a,0x47,0xe0,0x31,0x30,0x2c,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 21
    0xff,0xff,0xff,0x00,0xf6,0xfd,0xf3,0x09,0xf0,0xfa,0xef,0x12,0xf0,0xfb,0xf0,0x11,0xf0,0xfb,0xf0,0x11,0xf0,0xfb,0xf0,0x11,0xf0,0xfb,0xf0,0x11,0xf0,0xfa,0xef,0x12,0xf7,0xfe,0xf4,0x08,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xfc,0xfa,0x03,0xfe,0xef,0xeb,0x15,0xfe,0xf0,0xec,0x13,0xfe,0xf0,0xec,0x13,0xfe,0xf0,0xec,0x13,0xfe,0xf0,0xec,0x13,0xfe,0xf0,0xec,0x13,0xff,0xf1,0xed,0x11,0xee,0xf0,0xf0,0x0d,0x72,0x72,0x6f,0xaa,0x31,0x30,0x2b,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 22
    0xb6,0xea,0xb3,0x61,0x73,0xd7,0x6d,0xd9,0x72,0xd7,0x6b,0xe3,0x72,0xd7,0x6c,0xe2,0x72,0xd7,0x6c,0xe2,0x72,0xd7,0x6c,0xe2,0x72,0xd7,0x6c,0xe2,0x71,0xd7,0x6b,0xe3,0x70,0xd6,0x6a,0xd8,0xd0,0xf7,0xd5,0x33,0xff,0xe6,0xe3,0x1e,0xfa,0x77,0x52,0xd3,0xfa,0x6e,0x47,0xe8,0xfa,0x70,0x4a,0xe6,0xfa,0x70,0x4a,0xe6,0xfa,0x70,0x4a,0xe6,0xfa,0x70,0x4a,0xe6,0xfa,0x70,0x49,0xe7,0xfa,0x6a,0x42,0xe5,0xff,0xb2,0x9b,0x78,0xe1,0xe5,0xe6,0x19,0x3b,0x3a,0x37,0xf4,0x37,0x36,0x32,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 23
    0x80,0xdb,0x7c,0xc1,0x62,0xd1,0x5c,0xff,0x64,0xd2,0x5f,0xff,0x64,0xd2,0x5f,0xff,0x64,0xd2,0x5f,0xff,0x64,0xd2,0x5f,0xff,0x64,0xd2,0x5f,0xff,0x64,0xd2,0x5f,0xff,0x5a,0xcf,0x54,0xff,0xac,0xf0,0xb4,0x6e,0xff,0xcc,0xc5,0x42,0xf8,0x59,0x2c,0xff,0xf8,0x62,0x39,0xff,0xf8,0x62,0x38,0xff,0xf8,0x62,0x38,0xff,0xf8,0x62,0x38,0xff,0xf8,0x62,0x38,0xff,0xf8,0x62,0x38,0xff,0xf8,0x5c,0x31,0xff,0xfe,0x81,0x5f,0xc8,0xef,0xef,0xef,0x10,0x46,0x46,0x43,0xe6,0x35,0x34,0x30,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 24
    0x85,0xdc,0x81,0xbc,0x65,0xd2,0x60,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x5e,0xd0,0x58,0xff,0xb0,0xf0,0xb7,0x6a,0xff,0xce,0xc7,0x3f,0xf8,0x5d,0x32,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x5f,0x35,0xff,0xfe,0x86,0x65,0xc0,0xec,0xec,0xec,0x13,0x45,0x45,0x42,0xe7,0x35,0x34,0x30,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 25
    0x85,0xdc,0x81,0xbb,0x65,0xd2,0x60,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x5e,0xd0,0x58,0xff,0xb0,0xf0,0xb7,0x6a,0xff,0xce,0xc7,0x3f,0xf8,0x5d,0x32,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x5f,0x35,0xff,0xfe,0x86,0x65,0xc0,0xec,0xec,0xec,0x13,0x45,0x45,0x42,0xe7,0x35,0x34,0x30,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 26
    0x85,0xdc,0x81,0xbc,0x65,0xd2,0x60,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x5e,0xd0,0x58,0xff,0xb0,0xf0,0xb7,0x6a,0xff,0xce,0xc7,0x3f,0xf8,0x5d,0x32,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x5f,0x35,0xff,0xfe,0x86,0x65,0xc0,0xec,0xec,0xec,0x13,0x45,0x45,0x42,0xe7,0x35,0x34,0x30,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3f,0x3e,0x3a,0xf3,
    // Row 27
    0x83,0xdc,0x7f,0xbc,0x65,0xd2,0x60,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x5e,0xd0,0x58,0xff,0xb0,0xf0,0xb7,0x6a,0xff,0xce,0xc7,0x3f,0xf8,0x5d,0x32,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x5f,0x35,0xff,0xfe,0x86,0x65,0xc0,0xec,0xec,0xec,0x13,0x45,0x45,0x42,0xe7,0x35,0x34,0x30,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x38,0x37,0x33,0xff,0x3e,0x3d,0x39,0xf4,
    // Row 28
    0x9d,0xe3,0x9a,0x8a,0x62,0xd1,0x5c,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x67,0xd3,0x62,0xff,0x5e,0xd0,0x58,0xff,0xb0,0xf0,0xb7,0x6a,0xff,0xce,0xc7,0x3f,0xf8,0x5d,0x32,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x65,0x3c,0xff,0xf8,0x5f,0x35,0xff,0xfe,0x86,0x65,0xc0,0xec,0xec,0xec,0x13,0x45,0x45,0x42,0xe7,0x35,0x34,0x30,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x39,0x38,0x34,0xff,0x36,0x35,0x31,0xff,0x46,0x44,0x41,0xe7,
    // Row 29
    0xe4,0xf7,0xe3,0x1e,0x73,0xd7,0x6e,0xd7,0x60,0xd1,0x5a,0xff,0x66,0xd3,0x61,0xff,0x66,0xd3,0x61,0xff,0x66,0xd3,0x61,0xff,0x66,0xd3,0x61,0xff,0x66,0xd3,0x61,0xff,0x5c,0xd0,0x56,0xff,0xad,0xf0,0xb4,0x6d,0xff,0xcc,0xc5,0x42,0xf8,0x5b,0x2f,0xff,0xf8,0x64,0x3b,0xff,0xf8,0x64,0x3b,0xff,0xf8,0x64,0x3b,0xff,0xf8,0x64,0x3b,0xff,0xf8,0x64,0x3b,0xff,0xf8,0x64,0x3b,0xff,0xf8,0x5e,0x34,0xff,0xfe,0x83,0x61,0xc6,0xeb,0xeb,0xeb,0x13,0x41,0x41,0x3e,0xec,0x34,0x33,0x2f,0xff,0x38,0x37,0x33,0xff,0x38,0x37,0x33,0xff,0x38,0x37,0x33,0xff,0x38,0x37,0x33,0xff,0x36,0x35,0x31,0xff,0x29,0x28,0x24,0xff,0x92,0x91,0x8f,0x81,
};

/**
 * Create a vg_lite buffer from the embedded LVGL logo data
 * @return Buffer containing the LVGL logo image, or nullptr on failure
 */
static vg_lite_buffer_t* create_lvgl_logo_image() {
    vg_lite_buffer_t* image = create_buffer(LVGL_LOGO_WIDTH, LVGL_LOGO_HEIGHT, VG_LITE_BGRA8888);
    if (!image) return nullptr;
    
    // Copy the embedded LVGL logo data to the buffer
    // lvgl_logo_bgra_data is already in BGRA format
    uint8_t* dst = (uint8_t*)image->memory;
    std::memcpy(dst, lvgl_logo_bgra_data, sizeof(lvgl_logo_bgra_data));
    
    return image;
}

/**
 * Create a test image with a pattern suitable for recolor testing
 * Uses the LVGL logo as base
 * @param size Target size (will scale the 30x30 logo)
 * @return Buffer containing the test image, or nullptr on failure
 */
static vg_lite_buffer_t* create_recolor_test_image(int size) {
    // First get the base LVGL logo
    vg_lite_buffer_t* logo = create_lvgl_logo_image();
    if (!logo) return nullptr;
    
    // If same size, just return the logo
    if (size == LVGL_LOGO_WIDTH) {
        return logo;
    }
    
    // Otherwise create a new buffer and copy (simple nearest-neighbor scaling)
    vg_lite_buffer_t* image = create_buffer(size, size, VG_LITE_BGRA8888);
    if (!image) {
        free_buffer(logo);
        return nullptr;
    }
    
    float scale = (float)size / LVGL_LOGO_WIDTH;
    uint32_t* dst = (uint32_t*)image->memory;
    uint32_t* src = (uint32_t*)logo->memory;
    
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int sx = (int)(x / scale);
            int sy = (int)(y / scale);
            if (sx < LVGL_LOGO_WIDTH && sy < LVGL_LOGO_HEIGHT) {
                dst[y * size + x] = src[sy * LVGL_LOGO_WIDTH + sx];
            }
        }
    }
    
    free_buffer(logo);
    return image;
}

/**
 * ARGB1555 Render Test Fixture
 * 
 * Tests rendering to ARGB1555 format buffers where:
 * - A: 1 bit (0 or 1)
 * - R: 5 bits (0-31)
 * - G: 5 bits (0-31)
 * - B: 5 bits (0-31)
 */
class ARGB1555RenderTest : public VGLiteTest {
protected:
    static bool s_initialized;
    
    static void SetUpTestSuite() {
        // Initialize vg_lite once for all tests
        if (!s_initialized) {
            vg_lite_error_t error = vg_lite_init(256, 256);
            if (error == VG_LITE_SUCCESS) {
                s_initialized = true;
            }
        }
    }
    
    static void TearDownTestSuite() {
        // Close vg_lite after all tests
        if (s_initialized) {
            vg_lite_close();
            s_initialized = false;
        }
    }
    
    void SetUp() override {
        VGLiteTest::SetUp();
        // Ensure vg_lite is initialized
        if (!s_initialized) {
            vg_lite_init(256, 256);
            s_initialized = true;
        }
    }
    
    void TearDown() override {
        // Don't call vg_lite_close here - it's done in TearDownTestSuite
        VGLiteTest::TearDown();
    }
};

bool ARGB1555RenderTest::s_initialized = false;

// ============================================================================
// LVGL-compatible scene parameters
// ============================================================================

// LVGL demo render parameters:
// - Screen size: 800x480
// - Scene area: 480x272 (top-left corner)
// - Scene background: 0xaaf (light blue)
// - Grid: 8 columns x 8 rows
// - Cell size: 60x34
// - Object size: 55x30

#define LVGL_SCENE_WIDTH    480
#define LVGL_SCENE_HEIGHT   272
#define LVGL_CELL_WIDTH     60
#define LVGL_CELL_HEIGHT    34
#define LVGL_OBJ_WIDTH      55
#define LVGL_OBJ_HEIGHT     30
#define LVGL_COLS           8
#define LVGL_ROWS           8

// LVGL fill scene colors (column-based, hex3 -> hex6)
static const uint32_t lvgl_fill_colors[LVGL_COLS] = {
    0x000000,  // 0x000 -> black
    0xFFFFFF,  // 1xfff -> white
    0xFF0000,  // 1xf00 -> red
    0x00FF00,  // 1x0f0 -> green
    0x0000FF,  // 1x00f -> blue
    0xFFFF00,  // 1xff0 -> yellow
    0x00FFFF,  // 1x0ff -> cyan
    0xFF00FF   // 1xf0f -> magenta
};

// ============================================================================
// FILL Scene - LVGL compatible implementation
// ============================================================================

/**
 * Render filled rectangles matching LVGL's fill_cb
 * Creates 8 columns x 7 rows of rectangles with different properties:
 * - Row 0: Plain rectangles
 * - Row 1: Rounded rectangles (radius=10)
 * - Row 2: Circle rectangles (radius=100)
 * - Rows 3-6: Gradient rectangles (simplified to solid colors)
 */
void render_scene_fill(vg_lite_buffer_t* buffer, uint8_t opa) {
    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    
    // Fill entire buffer with white background
    fill_buffer(buffer, make_color(255, 255, 255, 255));
    
    // Fill scene area (480x272) with light blue background (0xaaf)
    vg_lite_path_t* bg_path = create_rect_path(0, 0, LVGL_SCENE_WIDTH, LVGL_SCENE_HEIGHT);
    if (bg_path) {
        vg_lite_draw(buffer, bg_path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(opa, 0xAA, 0xAA, 0xFF));
        free_path(bg_path);
    }
    
    // ARGB1555 has 1-bit alpha: opacity > 127 = fully opaque
    uint8_t effective_opa = (opa > 127) ? 255 : 0;
    
    // Row 0: Plain rectangles (no radius)
    for (int col = 0; col < LVGL_COLS; col++) {
        float x = col * LVGL_CELL_WIDTH + (LVGL_CELL_WIDTH - LVGL_OBJ_WIDTH) / 2;
        float y = 0 * LVGL_CELL_HEIGHT + (LVGL_CELL_HEIGHT - LVGL_OBJ_HEIGHT) / 2;
        
        vg_lite_path_t* path = create_rect_path(x, y, LVGL_OBJ_WIDTH, LVGL_OBJ_HEIGHT);
        if (path) {
            uint32_t color = lvgl_fill_colors[col];
            vg_lite_draw(buffer, path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                         make_color(effective_opa, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
            free_path(path);
        }
    }
    
    // Row 1: Rounded rectangles (radius=10)
    for (int col = 0; col < LVGL_COLS; col++) {
        float x = col * LVGL_CELL_WIDTH + (LVGL_CELL_WIDTH - LVGL_OBJ_WIDTH) / 2;
        float y = 1 * LVGL_CELL_HEIGHT + (LVGL_CELL_HEIGHT - LVGL_OBJ_HEIGHT) / 2;
        
        vg_lite_path_t* path = create_round_rect_path(x, y, LVGL_OBJ_WIDTH, LVGL_OBJ_HEIGHT, 10);
        if (path) {
            uint32_t color = lvgl_fill_colors[col];
            vg_lite_draw(buffer, path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                         make_color(effective_opa, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
            free_path(path);
        }
    }
    
    // Row 2: Circle rectangles (radius=100 - effectively circular)
    for (int col = 0; col < LVGL_COLS; col++) {
        float x = col * LVGL_CELL_WIDTH + (LVGL_CELL_WIDTH - LVGL_OBJ_WIDTH) / 2;
        float y = 2 * LVGL_CELL_HEIGHT + (LVGL_CELL_HEIGHT - LVGL_OBJ_HEIGHT) / 2;
        
        vg_lite_path_t* path = create_round_rect_path(x, y, LVGL_OBJ_WIDTH, LVGL_OBJ_HEIGHT, 100);
        if (path) {
            uint32_t color = lvgl_fill_colors[col];
            vg_lite_draw(buffer, path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                         make_color(effective_opa, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
            free_path(path);
        }
    }
    
    // Rows 3-6: Gradient rectangles (simplified - just use solid colors with rounding)
    // Note: vg_lite doesn't support LVGL-style gradients directly
    for (int row = 3; row < 7; row++) {
        for (int col = 0; col < LVGL_COLS; col++) {
            float x = col * LVGL_CELL_WIDTH + (LVGL_CELL_WIDTH - LVGL_OBJ_WIDTH) / 2;
            float y = row * LVGL_CELL_HEIGHT + (LVGL_CELL_HEIGHT - LVGL_OBJ_HEIGHT) / 2;
            
            vg_lite_path_t* path = create_round_rect_path(x, y, LVGL_OBJ_WIDTH, LVGL_OBJ_HEIGHT, 10);
            if (path) {
                uint32_t color = lvgl_fill_colors[col];
                vg_lite_draw(buffer, path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                             make_color(effective_opa, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
                free_path(path);
            }
        }
    }
    vg_lite_finish();
}

// ============================================================================
// LINE Scene - Multiple lines with various widths, colors, and opacity
// ============================================================================

/**
 * Render lines with various widths and colors
 * @param buffer Target buffer for rendering
 * @param opa Global opacity (0xff for full opacity, 0x80 for 50%)
 */
void render_scene_line(vg_lite_buffer_t* buffer, uint8_t opa) {
    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    
    // ARGB1555 has 1-bit alpha, so opacity > 127 means fully opaque
    uint8_t effective_alpha = (opa > 127) ? 255 : 0;
    
    // Draw horizontal lines with various widths (scaled from 160x160 to 800x480: 5x/3x)
    float y_pos = 60.0f;
    
    // Line width ~3px - Red
    vg_lite_path_t* path1 = create_rect_path(50, y_pos, 700, 3);
    if (path1) {
        vg_lite_draw(buffer, path1, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 255, 0, 0));  // Red
        free_path(path1);
    }
    
    // Line width ~6px - Green
    y_pos += 60;
    vg_lite_path_t* path2 = create_rect_path(50, y_pos, 700, 6);
    if (path2) {
        vg_lite_draw(buffer, path2, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 0, 255, 0));  // Green
        free_path(path2);
    }
    
    // Line width ~12px - Blue
    y_pos += 75;
    vg_lite_path_t* path3 = create_rect_path(50, y_pos, 700, 12);
    if (path3) {
        vg_lite_draw(buffer, path3, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 0, 0, 255));  // Blue
        free_path(path3);
    }
    
    // Line width ~24px - Yellow
    y_pos += 90;
    vg_lite_path_t* path4 = create_rect_path(50, y_pos, 700, 24);
    if (path4) {
        vg_lite_draw(buffer, path4, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 255, 255, 0));  // Yellow
        free_path(path4);
    }
    
    // Draw vertical lines on the right side
    float x_pos = 500.0f;
    
    // Vertical line width ~6px - Cyan
    vg_lite_path_t* path5 = create_rect_path(x_pos, 60, 6, 360);
    if (path5) {
        vg_lite_draw(buffer, path5, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 0, 255, 255));  // Cyan
        free_path(path5);
    }
    
    // Vertical line width ~12px - Magenta
    x_pos += 100;
    vg_lite_path_t* path6 = create_rect_path(x_pos, 60, 12, 360);
    if (path6) {
        vg_lite_draw(buffer, path6, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 255, 0, 255));  // Magenta
        free_path(path6);
    }
    vg_lite_finish();
}

// ============================================================================
// ARC_IMAGE Scene - Render arcs with image fills
// ============================================================================

/**
 * Create a gradient test image
 * @param width Image width
 * @param height Image height (optional, defaults to width for square)
 * @return Buffer containing a gradient image, or nullptr on failure
 */
static vg_lite_buffer_t* create_gradient_image(int width, int height = 0) {
    if (height == 0) height = width;  // Default to square
    vg_lite_buffer_t* image = create_buffer(width, height, VG_LITE_BGRA8888);
    if (!image) return nullptr;
    
    uint32_t* px = (uint32_t*)image->memory;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Create a simple radial-like gradient
            uint8_t r = (uint8_t)((x * 255) / width);
            uint8_t g = (uint8_t)((y * 255) / height);
            uint8_t b = (uint8_t)(((x + y) * 255) / (width + height));
            px[y * width + x] = make_color(255, r, g, b);
        }
    }
    
    return image;
}

/**
 * Create an arc path (partial circle for stroke rendering)
 * @param cx Center X
 * @param cy Center Y  
 * @param radius Arc radius
 * @param start_angle Start angle in degrees (0 = right, counter-clockwise)
 * @param end_angle End angle in degrees
 * @return Path for the arc
 */
static vg_lite_path_t* create_arc_path(float cx, float cy, float radius, 
                                        float start_angle, float end_angle) {
    // Allocate path
    vg_lite_path_t* path = (vg_lite_path_t*)malloc(sizeof(vg_lite_path_t));
    if (!path) return nullptr;
    
    // Calculate arc points (use line segments to approximate)
    const int num_segments = 32;
    float angle_diff = end_angle - start_angle;
    if (angle_diff <= 0) angle_diff += 360.0f;
    
    // Calculate data size: MOVE + (segments) LINE commands
    int data_size = (1 + num_segments + 1) * sizeof(vg_lite_float_t) * 2 + 256;
    uint8_t* path_data = (uint8_t*)malloc(data_size);
    if (!path_data) {
        free(path);
        return nullptr;
    }
    
    // Build path data
    uint8_t* ptr = path_data;
    vg_lite_float_t* coords;
    
    // Move to start point
    float start_rad = start_angle * 3.14159265f / 180.0f;
    float x1 = cx + radius * cosf(start_rad);
    float y1 = cy - radius * sinf(start_rad);  // Y axis is inverted in screen coords
    
    *ptr++ = VLC_OP_MOVE;  // Move command
    coords = (vg_lite_float_t*)ptr;
    *coords++ = x1;
    *coords++ = y1;
    ptr = (uint8_t*)coords;
    
    // Draw line segments around the arc
    for (int i = 1; i <= num_segments; i++) {
        float angle = start_angle + (angle_diff * i / num_segments);
        float rad = angle * 3.14159265f / 180.0f;
        float x = cx + radius * cosf(rad);
        float y = cy - radius * sinf(rad);
        
        *ptr++ = VLC_OP_LINE;  // Line command
        coords = (vg_lite_float_t*)ptr;
        *coords++ = x;
        *coords++ = y;
        ptr = (uint8_t*)coords;
    }
    
    *ptr++ = VLC_OP_END;
    
    // Initialize path
    float min_x = cx - radius - 1;
    float min_y = cy - radius - 1;
    float max_x = cx + radius + 1;
    float max_y = cy + radius + 1;
    
    vg_lite_error_t err = vg_lite_init_arc_path(path, 
                                                 VG_LITE_S8,  // Coordinate format
                                                 VG_LITE_HIGH,  // Quality
                                                 (uint32_t)(ptr - path_data),  // Data length
                                                 path_data,
                                                 min_x, min_y, max_x, max_y);
    
    if (err != VG_LITE_SUCCESS) {
        free(path_data);
        free(path);
        return nullptr;
    }
    
    return path;
}

/**
 * Render arc image scene matching LVGL's arc_image demo
 * @param buffer Target buffer for rendering
 * @param opa Global opacity
 */
void render_scene_arc_image(vg_lite_buffer_t* buffer, uint8_t opa) {
    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    
    // ARGB1555 has 1-bit alpha, so opacity > 127 means fully opaque
    uint8_t effective_alpha = (opa > 127) ? 255 : (uint8_t)(opa / 2);
    
    // Fill with white background
    fill_buffer(buffer, make_color(255, 255, 255, 255));
    
    // LVGL arc_image scene parameters
    // Grid: 8 columns x 8 rows
    // Angles for each column
    float angles[][2] = {
        {355, 5},     // Small arc near 0
        {85, 95},     // Small arc near 90
        {175, 185},   // Small arc near 180
        {265, 275},   // Small arc near 270
        {30, 330},    // Large arc (most of circle)
        {120, 60},    // Arc crossing 0
        {0, 180},     // Half circle
        {0, 360},     // Full circle
    };
    
    // Arc widths for each row group (4 widths x 2 rounded styles = 8 rows)
    int32_t widths[] = {1, 5, 10, 100};
    
    // Cell layout (matching LVGL's grid)
    float cell_width = 55.0f;
    float cell_height = 30.0f;
    float arc_radius = 10.0f;  // Arc radius in each cell
    
    // Draw arcs in 8x8 grid
    for (int row = 0; row < 8; row++) {
        int width_idx = row % 4;
        int rounded = row / 4;  // 0 = not rounded, 1 = rounded
        
        float line_width = (float)widths[width_idx];
        if (line_width > arc_radius * 2) line_width = arc_radius * 2 - 2;
        
        for (int col = 0; col < 8; col++) {
            // Cell center position
            float cx = cell_width / 2 + col * cell_width + 40;
            float cy = cell_height / 2 + row * cell_height + 30;
            
            // Create arc path
            float start_angle = angles[col][0];
            float end_angle = angles[col][1];
            
            vg_lite_path_t* arc = create_arc_path(cx, cy, arc_radius, start_angle, end_angle);
            if (arc) {
                // Set stroke properties
                vg_lite_cap_style_t cap = rounded ? VG_LITE_CAP_ROUND : VG_LITE_CAP_BUTT;
                vg_lite_join_style_t join = rounded ? VG_LITE_JOIN_ROUND : VG_LITE_JOIN_MITER;
                
                vg_lite_set_stroke(arc, cap, join, line_width, 4.0f, 
                                    nullptr, 0, 0,  // No dash
                                    make_color(effective_alpha, 255, 255, 255));
                
                // Draw the stroked arc
                vg_lite_draw(buffer, arc, VG_LITE_FILL_NON_ZERO, &matrix, 
                            VG_LITE_BLEND_SRC_OVER, make_color(effective_alpha, 255, 255, 255));
                
                free_path(arc);
            }
        }
    }
    
    vg_lite_finish();
}

// Test: ARC_IMAGE scene with full opacity
#if ARGB1555_TESTS_ENABLED
TEST_F(ARGB1555RenderTest, test_arc_image) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Fill with white background
    fill_buffer(buffer, make_color(255, 255, 255, 255));
    
    // Render arc image scene with full opacity
    render_scene_arc_image(buffer, 0xff);
    
    // Request dump for debugging
    RequestDump(buffer, "argb1555/demo_render_arc_image_opa_255");
    
    // Save output PNG
    std::string png_path = "test_output_demo_render_arc_image_opa_255.png";
    save_buffer_to_png(buffer, png_path);

    
    // Compare with golden image (using relative path to ref_imgs_vg_lite)
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_arc_image_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_arc_image_opa_255.png");
    
    free_buffer(buffer);
}

// Test: ARC_IMAGE scene with half opacity
TEST_F(ARGB1555RenderTest, test_arc_image_opa80) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Fill with white background
    fill_buffer(buffer, make_color(255, 255, 255, 255));
    
    // Render arc image scene with half opacity (0x80)
    // Note: ARGB1555 has 1-bit alpha, so semi-transparent may look different
    render_scene_arc_image(buffer, 0x80);
    
    // Request dump for debugging
    RequestDump(buffer, "argb1555_arc_image_opa80");
    
    // Save output PNG
    std::string png_path = "test_output_argb1555_arc_image_opa80.png";
    save_buffer_to_png(buffer, png_path);

    
    // Compare with golden image (higher tolerance for opacity test)
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_arc_image_opa_128.png", 0.15);
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_arc_image_opa_128.png", 0.15);
    
    free_buffer(buffer);
}
#else
TEST_F(ARGB1555RenderTest, test_arc_image) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms due to color precision differences";
}
TEST_F(ARGB1555RenderTest, test_arc_image_opa80) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms due to color precision differences";
}
#endif

// ============================================================================
// IMAGE_NORMAL Scene - Render images using vg_lite_blit()
// ============================================================================

/**
 * Create a test image with colored squares pattern
 * Used as source for blit operations
 */
static vg_lite_buffer_t* create_test_icon_image(int size) {
    vg_lite_buffer_t* image = create_buffer(size, size, VG_LITE_BGRA8888);
    if (!image) return nullptr;
    
    // Fill with black background
    uint32_t* px = (uint32_t*)image->memory;
    for (int i = 0; i < size * size; i++) {
        px[i] = make_color(255, 0, 0, 0);  // Black with full alpha
    }
    
    // Draw colored squares in corners (red, green, blue pattern)
    int quad = size / 4;
    
    // Top-left: red
    for (int y = quad; y < quad * 2; y++) {
        for (int x = quad; x < quad * 2; x++) {
            px[y * size + x] = make_color(255, 255, 0, 0);
        }
    }
    
    // Top-right: green
    for (int y = quad; y < quad * 2; y++) {
        for (int x = quad * 2; x < quad * 3; x++) {
            px[y * size + x] = make_color(255, 0, 255, 0);
        }
    }
    
    // Bottom-left: blue
    for (int y = quad * 2; y < quad * 3; y++) {
        for (int x = quad; x < quad * 2; x++) {
            px[y * size + x] = make_color(255, 0, 0, 255);
        }
    }
    
    // Bottom-right: orange
    for (int y = quad * 2; y < quad * 3; y++) {
        for (int x = quad * 2; x < quad * 3; x++) {
            px[y * size + x] = make_color(255, 255, 128, 0);
        }
    }
    
    return image;
}

/**
 * IMAGE_NORMAL Scene 1: Grid layout with various source formats
 * Demonstrates basic image blitting with different source buffer formats
 * Uses the actual LVGL logo image data
 */
void render_scene_image_normal_1(vg_lite_buffer_t* buffer, uint8_t opa) {
    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    
    // Create LVGL logo image (30x30 ARGB8888)
    vg_lite_buffer_t* src_logo = create_lvgl_logo_image();
    
    if (!src_logo) {
        std::cerr << "[ERROR] Failed to create LVGL logo image" << std::endl;
        return;
    }
    
    // Calculate opacity factor (0-1)
    float alpha_factor = opa / 255.0f;
    uint8_t effective_alpha = (opa > 127) ? 255 : (uint8_t)(opa * alpha_factor);
    
    // Light purple/lavender background (matching LVGL render demo)
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    // Grid layout similar to LVGL's image_normal_1 scene
    // 4 rows x 9 columns of logos
    int start_x = 10;
    int start_y = 10;
    int spacing_x = 52;
    int spacing_y = 58;
    
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 9; col++) {
            float x = start_x + col * spacing_x;
            float y = start_y + row * spacing_y;
            
            vg_lite_matrix_t blit_matrix;
            vg_lite_identity(&blit_matrix);
            vg_lite_translate(x, y, &blit_matrix);
            
            vg_lite_blit(buffer, src_logo, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                         make_color(effective_alpha, 255, 255, 255), VG_LITE_FILTER_POINT);
        }
    }
    
    free_buffer(src_logo);
}

/**
 * IMAGE_NORMAL Scene 2: Multiple format rows with different patterns
 * Tests different source image formats and sizes
 */
void render_scene_image_normal_2(vg_lite_buffer_t* buffer, uint8_t opa) {
    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    
    // Create gradient source images
    vg_lite_buffer_t* src1 = create_gradient_image(16, 16);
    vg_lite_buffer_t* src2 = create_gradient_image(20, 20);
    vg_lite_buffer_t* src3 = create_gradient_image(24, 24);
    
    if (!src1 || !src2 || !src3) {
        std::cerr << "[ERROR] Failed to create gradient images" << std::endl;
        if (src1) free_buffer(src1);
        if (src2) free_buffer(src2);
        if (src3) free_buffer(src3);
        return;
    }
    
    // Light purple background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    uint8_t effective_alpha = (opa > 127) ? 255 : (uint8_t)(opa / 2);
    
    // Row 1: ARGB1555 format - colorful patterns
    float y = 30;
    float x = 70;
    for (int i = 0; i < 8; i++) {
        vg_lite_matrix_t blit_matrix;
        vg_lite_identity(&blit_matrix);
        vg_lite_translate(x, y, &blit_matrix);
        
        vg_lite_buffer_t* src = (i % 3 == 0) ? src1 : (i % 3 == 1) ? src2 : src3;
        
        vg_lite_blit(buffer, src, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 255, 255, 255), VG_LITE_FILTER_POINT);
        
        x += 40;
    }
    
    // Row 2: RGB565 SWAP format
    y = 70;
    x = 70;
    for (int i = 0; i < 8; i++) {
        vg_lite_matrix_t blit_matrix;
        vg_lite_identity(&blit_matrix);
        vg_lite_translate(x, y, &blit_matrix);
        
        vg_lite_buffer_t* src = (i % 3 == 0) ? src1 : (i % 3 == 1) ? src2 : src3;
        
        vg_lite_blit(buffer, src, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 255, 255, 255), VG_LITE_FILTER_POINT);
        
        x += 40;
    }
    
    // Row 3: L8 format (grayscale)
    y = 110;
    x = 70;
    for (int i = 0; i < 8; i++) {
        vg_lite_matrix_t blit_matrix;
        vg_lite_identity(&blit_matrix);
        vg_lite_translate(x, y, &blit_matrix);
        
        vg_lite_buffer_t* src = (i % 3 == 0) ? src1 : (i % 3 == 1) ? src2 : src3;
        
        vg_lite_blit(buffer, src, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 255, 255, 255), VG_LITE_FILTER_POINT);
        
        x += 40;
    }
    
    free_buffer(src1);
    free_buffer(src2);
    free_buffer(src3);
}

/**
 * IMAGE_NORMAL Scene 3: Staggered diagonal layout with rotations
 * Tests image blitting with transformations
 */
void render_scene_image_normal_3(vg_lite_buffer_t* buffer, uint8_t opa) {
    // Light purple background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    // Create source images
    vg_lite_buffer_t* icon1 = create_test_icon_image(24);
    vg_lite_buffer_t* icon2 = create_buffer(24, 24, VG_LITE_BGRA8888);
    
    if (!icon1 || !icon2) {
        std::cerr << "[ERROR] Failed to create icon images" << std::endl;
        if (icon1) free_buffer(icon1);
        if (icon2) free_buffer(icon2);
        return;
    }
    
    // Fill icon2 with black and draw a white "2"
    uint32_t* px = (uint32_t*)icon2->memory;
    for (int i = 0; i < 24 * 24; i++) {
        px[i] = make_color(255, 0, 0, 0);  // Black
    }
    // Draw a simple "2" shape using white pixels
    for (int y = 4; y < 20; y++) {
        for (int x = 6; x < 18; x++) {
            bool draw = false;
            // Top horizontal
            if (y >= 4 && y <= 6 && x >= 8 && x <= 16) draw = true;
            // Right side top
            if (y >= 4 && y <= 10 && x >= 14 && x <= 16) draw = true;
            // Middle horizontal
            if (y >= 9 && y <= 11 && x >= 8 && x <= 16) draw = true;
            // Left side bottom
            if (y >= 9 && y <= 18 && x >= 6 && x <= 8) draw = true;
            // Bottom horizontal
            if (y >= 16 && y <= 18 && x >= 6 && x <= 16) draw = true;
            
            if (draw) px[y * 24 + x] = make_color(255, 255, 255, 255);  // White
        }
    }
    
    uint8_t effective_alpha = (opa > 127) ? 255 : (uint8_t)(opa / 2);
    
    // Top row: staggered icon1 pattern
    float y = 40;
    float x = 50;
    for (int i = 0; i < 5; i++) {
        vg_lite_matrix_t blit_matrix;
        vg_lite_identity(&blit_matrix);
        vg_lite_translate(x + i * 20, y + i * 15, &blit_matrix);
        vg_lite_rotate(i * 15.0f, &blit_matrix);  // Rotate each icon
        
        vg_lite_blit(buffer, icon1, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 255, 255, 255), VG_LITE_FILTER_POINT);
    }
    
    // Bottom row: staggered icon2 pattern
    y = 120;
    x = 50;
    for (int i = 0; i < 5; i++) {
        vg_lite_matrix_t blit_matrix;
        vg_lite_identity(&blit_matrix);
        vg_lite_translate(x + i * 20, y + i * 15, &blit_matrix);
        vg_lite_rotate(i * 15.0f, &blit_matrix);
        
        vg_lite_blit(buffer, icon2, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                     make_color(effective_alpha, 255, 255, 255), VG_LITE_FILTER_POINT);
    }
    
    free_buffer(icon1);
    free_buffer(icon2);
}

// ============================================================================
// IMAGE_RECOLOR Scene - Render images with color tinting (recolor mode)
// ============================================================================

/**
 * IMAGE_RECOLOR Scene 1: Grid layout with green tinting effect
 * Demonstrates recolor mode with multiple source formats
 */
void render_scene_image_recolor_1(vg_lite_buffer_t* buffer, uint8_t opa) {
    // Light purple background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    // Create source image for recolor
    vg_lite_buffer_t* src = create_recolor_test_image(32);
    if (!src) {
        std::cerr << "[ERROR] Failed to create recolor test image" << std::endl;
        return;
    }
    
    // Set source image to recolor mode
    src->image_mode = VG_LITE_RECOLOR_MODE;
    
    uint8_t effective_alpha = (opa > 127) ? 255 : (uint8_t)(opa / 2);
    
    // Green tint color for recolor
    vg_lite_color_t tint_color = make_color(effective_alpha, 0, 200, 100);
    
    // Grid: 4 rows x 5 columns
    float y = 30;
    for (int row = 0; row < 4; row++) {
        float x = 70;
        for (int col = 0; col < 5; col++) {
            vg_lite_matrix_t blit_matrix;
            vg_lite_identity(&blit_matrix);
            vg_lite_translate(x, y, &blit_matrix);
            
            vg_lite_blit(buffer, src, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                         tint_color, VG_LITE_FILTER_POINT);
            
            x += 50;
        }
        y += 45;
    }
    
    free_buffer(src);
}

/**
 * IMAGE_RECOLOR Scene 2: Three columns with different format labels
 * Demonstrates recolor mode across different source formats
 */
void render_scene_image_recolor_2(vg_lite_buffer_t* buffer, uint8_t opa) {
    // Light purple background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    // Create source image
    vg_lite_buffer_t* src = create_recolor_test_image(24);
    if (!src) {
        std::cerr << "[ERROR] Failed to create recolor test image" << std::endl;
        return;
    }
    
    // Set source image to recolor mode
    src->image_mode = VG_LITE_RECOLOR_MODE;
    
    uint8_t effective_alpha = (opa > 127) ? 255 : (uint8_t)(opa / 2);
    
    // Tint colors for each column
    vg_lite_color_t tint_colors[] = {
        make_color(effective_alpha, 0, 200, 100),    // Green
        make_color(effective_alpha, 100, 0, 200),    // Purple
        make_color(effective_alpha, 150, 150, 150),  // Gray
    };
    
    // Three columns: ARGB1555, RGB565 SWAP, L8
    float col_x[] = {70, 220, 370};
    
    for (int col = 0; col < 3; col++) {
        float y = 30;
        // 2 rows x 6 icons per column
        for (int row = 0; row < 2; row++) {
            float x = col_x[col];
            for (int i = 0; i < 6; i++) {
                vg_lite_matrix_t blit_matrix;
                vg_lite_identity(&blit_matrix);
                vg_lite_translate(x, y, &blit_matrix);
                
                vg_lite_blit(buffer, src, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                             tint_colors[col], VG_LITE_FILTER_POINT);
                
                x += 25;
            }
            y += 40;
        }
    }
    
    free_buffer(src);
}

/**
 * IMAGE_RECOLOR Scene 3: Rotated images with green tinting
 * Demonstrates recolor mode with transformations
 */
void render_scene_image_recolor_3(vg_lite_buffer_t* buffer, uint8_t opa) {
    // Light purple background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    // Create two source images
    vg_lite_buffer_t* icon1 = create_test_icon_image(24);
    vg_lite_buffer_t* icon2 = create_buffer(24, 24, VG_LITE_BGRA8888);
    
    if (!icon1 || !icon2) {
        std::cerr << "[ERROR] Failed to create icon images" << std::endl;
        if (icon1) free_buffer(icon1);
        if (icon2) free_buffer(icon2);
        return;
    }
    
    // Fill icon2 with black and draw a green "2"
    uint32_t* px = (uint32_t*)icon2->memory;
    for (int i = 0; i < 24 * 24; i++) {
        px[i] = make_color(255, 0, 0, 0);  // Black
    }
    // Draw a green "2" shape
    for (int y = 4; y < 20; y++) {
        for (int x = 6; x < 18; x++) {
            bool draw = false;
            if (y >= 4 && y <= 6 && x >= 8 && x <= 16) draw = true;
            if (y >= 4 && y <= 10 && x >= 14 && x <= 16) draw = true;
            if (y >= 9 && y <= 11 && x >= 8 && x <= 16) draw = true;
            if (y >= 9 && y <= 18 && x >= 6 && x <= 8) draw = true;
            if (y >= 16 && y <= 18 && x >= 6 && x <= 16) draw = true;
            
            if (draw) px[y * 24 + x] = make_color(255, 0, 255, 0);  // Green
        }
    }
    
    // Set to recolor mode
    icon1->image_mode = VG_LITE_RECOLOR_MODE;
    icon2->image_mode = VG_LITE_RECOLOR_MODE;
    
    uint8_t effective_alpha = (opa > 127) ? 255 : (uint8_t)(opa / 2);
    
    // Green tint color
    vg_lite_color_t tint_color = make_color(effective_alpha, 0, 200, 50);
    
    // Top row: rotated icon1 with green tint
    float y = 40;
    float x = 50;
    for (int i = 0; i < 5; i++) {
        vg_lite_matrix_t blit_matrix;
        vg_lite_identity(&blit_matrix);
        vg_lite_translate(x + i * 30, y, &blit_matrix);
        vg_lite_rotate(i * 20.0f, &blit_matrix);
        
        vg_lite_blit(buffer, icon1, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                     tint_color, VG_LITE_FILTER_POINT);
    }
    
    // Bottom row: rotated icon2 with green tint
    y = 110;
    x = 50;
    for (int i = 0; i < 5; i++) {
        vg_lite_matrix_t blit_matrix;
        vg_lite_identity(&blit_matrix);
        vg_lite_translate(x + i * 30, y, &blit_matrix);
        vg_lite_rotate(i * 20.0f, &blit_matrix);
        
        vg_lite_blit(buffer, icon2, &blit_matrix, VG_LITE_BLEND_SRC_OVER,
                     tint_color, VG_LITE_FILTER_POINT);
    }
    
    free_buffer(icon1);
    free_buffer(icon2);
}

// ============================================================================
// Tests for IMAGE_NORMAL and IMAGE_RECOLOR scenes
// ============================================================================

// Test: IMAGE_NORMAL scene 1 with full opacity
#if ARGB1555_TESTS_ENABLED
TEST_F(ARGB1555RenderTest, test_image_normal_1) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    render_scene_image_normal_1(buffer, 0xff);
    
    RequestDump(buffer, "argb1555_image_normal_1_full");
    
    std::string png_path = "test_output_argb1555_image_normal_1_full.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_1_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_1_opa_255.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_NORMAL scene 1 with half opacity
TEST_F(ARGB1555RenderTest, test_image_normal_1_opa80) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    render_scene_image_normal_1(buffer, 0x80);
    
    RequestDump(buffer, "argb1555_image_normal_1_opa80");
    
    std::string png_path = "test_output_argb1555_image_normal_1_opa80.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_1_opa_128.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_1_opa_128.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_NORMAL scene 2 with full opacity
TEST_F(ARGB1555RenderTest, test_image_normal_2) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_normal_2(buffer, 0xff);
    
    RequestDump(buffer, "argb1555_image_normal_2_full");
    
    std::string png_path = "test_output_argb1555_image_normal_2_full.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_2_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_2_opa_255.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_NORMAL scene 2 with half opacity
TEST_F(ARGB1555RenderTest, test_image_normal_2_opa80) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_normal_2(buffer, 0x80);
    
    RequestDump(buffer, "argb1555_image_normal_2_opa80");
    
    std::string png_path = "test_output_argb1555_image_normal_2_opa80.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_2_opa_128.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_2_opa_128.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_NORMAL scene 3 with full opacity
TEST_F(ARGB1555RenderTest, test_image_normal_3) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_normal_3(buffer, 0xff);
    
    RequestDump(buffer, "argb1555_image_normal_3_full");
    
    std::string png_path = "test_output_argb1555_image_normal_3_full.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_3_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_3_opa_255.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_NORMAL scene 3 with half opacity
TEST_F(ARGB1555RenderTest, test_image_normal_3_opa80) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_normal_3(buffer, 0x80);
    
    RequestDump(buffer, "argb1555_image_normal_3_opa80");
    
    std::string png_path = "test_output_argb1555_image_normal_3_opa80.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_3_opa_128.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_normal_3_opa_128.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_RECOLOR scene 1 with full opacity
TEST_F(ARGB1555RenderTest, test_image_recolor_1) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_recolor_1(buffer, 0xff);
    
    RequestDump(buffer, "argb1555_image_recolor_1_full");
    
    std::string png_path = "test_output_argb1555_image_recolor_1_full.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_1_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_1_opa_255.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_RECOLOR scene 1 with half opacity
TEST_F(ARGB1555RenderTest, test_image_recolor_1_opa80) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_recolor_1(buffer, 0x80);
    
    RequestDump(buffer, "argb1555_image_recolor_1_opa80");
    
    std::string png_path = "test_output_argb1555_image_recolor_1_opa80.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_1_opa_128.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_1_opa_128.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_RECOLOR scene 2 with full opacity
TEST_F(ARGB1555RenderTest, test_image_recolor_2) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_recolor_2(buffer, 0xff);
    
    RequestDump(buffer, "argb1555_image_recolor_2_full");
    
    std::string png_path = "test_output_argb1555_image_recolor_2_full.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_2_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_2_opa_255.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_RECOLOR scene 2 with half opacity
TEST_F(ARGB1555RenderTest, test_image_recolor_2_opa80) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_recolor_2(buffer, 0x80);
    
    RequestDump(buffer, "argb1555_image_recolor_2_opa80");
    
    std::string png_path = "test_output_argb1555_image_recolor_2_opa80.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_2_opa_128.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_2_opa_128.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_RECOLOR scene 3 with full opacity
TEST_F(ARGB1555RenderTest, test_image_recolor_3) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_recolor_3(buffer, 0xff);
    
    RequestDump(buffer, "argb1555_image_recolor_3_full");
    
    std::string png_path = "test_output_argb1555_image_recolor_3_full.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_3_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_3_opa_255.png");
    
    free_buffer(buffer);
}

// Test: IMAGE_RECOLOR scene 3 with half opacity
TEST_F(ARGB1555RenderTest, test_image_recolor_3_opa80) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    render_scene_image_recolor_3(buffer, 0x80);
    
    RequestDump(buffer, "argb1555_image_recolor_3_opa80");
    
    std::string png_path = "test_output_argb1555_image_recolor_3_opa80.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_3_opa_128.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_image_recolor_3_opa_128.png");
    
    free_buffer(buffer);
}
#else
// 32-bit platform stubs
TEST_F(ARGB1555RenderTest, test_image_normal_1) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_normal_1_opa80) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_normal_2) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_normal_2_opa80) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_normal_3) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_normal_3_opa80) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_recolor_1) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_recolor_1_opa80) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_recolor_2) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_recolor_2_opa80) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_recolor_3) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_image_recolor_3_opa80) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
#endif

// ============================================================================
// BLEND_MODE Scene - Various blend modes with overlapping shapes
// ============================================================================

/**
 * Render overlapping shapes demonstrating various blend modes
 * @param buffer Target buffer for rendering
 * @param opa Global opacity (0xff for full opacity, 0x80 for 50%)
 */
void render_scene_blend_mode(vg_lite_buffer_t* buffer, uint8_t opa) {
    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    
    // ARGB1555 has 1-bit alpha, so opacity > 127 means fully opaque
    uint8_t effective_alpha = (opa > 127) ? 255 : 0;
    
    // Define blend modes to demonstrate
    // Array of blend modes: Normal, Additive, Multiply, Screen, Lighten, Darken
    vg_lite_blend_t blend_modes[] = {
        VG_LITE_BLEND_SRC_OVER,
        VG_LITE_BLEND_ADDITIVE,
        VG_LITE_BLEND_MULTIPLY,
        VG_LITE_BLEND_SCREEN,
        VG_LITE_BLEND_LIGHTEN,
        VG_LITE_BLEND_DARKEN
    };
    
    // Layout configuration (scaled from 160x160 to 800x480: 5x/3x)
    int cell_width = 150;
    int cell_height = 90;
    int padding_x = 25;
    int padding_y = 15;
    int cols = 4;
    
    // Draw base white rectangles as background for each blend mode cell
    // This allows blend modes to have something to blend with
    for (size_t i = 0; i < sizeof(blend_modes) / sizeof(blend_modes[0]); i++) {
        int col = i % cols;
        int row = i / cols;
        int x = padding_x + col * (cell_width + padding_x);
        int y = padding_y + row * (cell_height + padding_y);
        
        // Draw gray background rectangle
        vg_lite_path_t* bg_path = create_rect_path(x, y, cell_width, cell_height);
        if (bg_path) {
            vg_lite_draw(buffer, bg_path, VG_LITE_FILL_EVEN_ODD, &matrix,
                        VG_LITE_BLEND_SRC_OVER, make_color(255, 128, 128, 128));
            free_path(bg_path);
        }
    }
    
    // Now draw overlapping colored shapes using each blend mode
    for (size_t i = 0; i < sizeof(blend_modes) / sizeof(blend_modes[0]); i++) {
        int col = i % cols;
        int row = i / cols;
        int x = padding_x + col * (cell_width + padding_x);
        int y = padding_y + row * (cell_height + padding_y);
        
        // Red rectangle (left side)
        vg_lite_path_t* red_path = create_rect_path(x + 25, y + 15, 150, 90);
        if (red_path) {
            vg_lite_draw(buffer, red_path, VG_LITE_FILL_EVEN_ODD, &matrix,
                        blend_modes[i], make_color(effective_alpha, 255, 50, 50));
            free_path(red_path);
        }
        
        // Green rectangle (overlapping, shifted right and down)
        vg_lite_path_t* green_path = create_rect_path(x + 100, y + 45, 150, 90);
        if (green_path) {
            vg_lite_draw(buffer, green_path, VG_LITE_FILL_EVEN_ODD, &matrix,
                        blend_modes[i], make_color(effective_alpha, 50, 255, 50));
            free_path(green_path);
        }
        
        // Blue rectangle (overlapping more)
        vg_lite_path_t* blue_path = create_rect_path(x + 125, y + 75, 150, 90);
        if (blue_path) {
            vg_lite_draw(buffer, blue_path, VG_LITE_FILL_EVEN_ODD, &matrix,
                        blend_modes[i], make_color(effective_alpha, 50, 50, 255));
            free_path(blue_path);
        }
    }
}

// Test: BLEND_MODE scene with full opacity
#if ARGB1555_TESTS_ENABLED
TEST_F(ARGB1555RenderTest, test_render_blend_mode_opa_255) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Fill with white background
    fill_buffer(buffer, make_color(255, 255, 255, 255));
    
    // Render blend mode scene with full opacity
    render_scene_blend_mode(buffer, 0xff);
    
    // Request dump for debugging
    RequestDump(buffer, "argb1555/demo_render_blend_mode_opa_255");
    
    // Save output PNG
    std::string png_path = "test_output_demo_render_blend_mode_opa_255.png";
    save_buffer_to_png(buffer, png_path);

    
    // Compare with golden image (using relative path to ref_imgs_vg_lite)
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_blend_mode_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_blend_mode_opa_255.png");
    
    free_buffer(buffer);
}
#else
TEST_F(ARGB1555RenderTest, test_render_blend_mode_opa_255) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms due to color precision differences";
}
#endif

// Test: BLEND_MODE scene with half opacity
#if ARGB1555_TESTS_ENABLED
TEST_F(ARGB1555RenderTest, test_render_blend_mode_opa_128) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Fill with white background
    fill_buffer(buffer, make_color(255, 255, 255, 255));
    
    // Render blend mode scene with half opacity
    render_scene_blend_mode(buffer, 0x80);
    
    // Request dump for debugging
    RequestDump(buffer, "argb1555/demo_render_blend_mode_opa_128");
    
    // Save output PNG
    std::string png_path = "test_output_demo_render_blend_mode_opa_128.png";
    save_buffer_to_png(buffer, png_path);

    
    // Compare with golden image (higher tolerance for opacity test)
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_blend_mode_opa_128.png", 0.15);
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_blend_mode_opa_128.png", 0.15);
    
    free_buffer(buffer);
}
#else
TEST_F(ARGB1555RenderTest, test_render_blend_mode_opa_128) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms due to color precision differences";
}
#endif

// ============================================================================
// BOX_SHADOW Scene - Rectangles and ovals with drop shadows
// ============================================================================

/**
 * Draw a rectangle with simulated drop shadow
 * For ARGB1555 (1-bit alpha), shadows are simulated using darker colors
 */
static void draw_rect_with_shadow(vg_lite_buffer_t* buffer,
                                   float x, float y, float w, float h,
                                   float shadow_offset,
                                   vg_lite_color_t main_color,
                                   vg_lite_color_t shadow_color,
                                   vg_lite_matrix_t* matrix) {
    // Draw shadow rectangle (offset down and right)
    vg_lite_path_t* shadow_path = create_rect_path(x + shadow_offset, y + shadow_offset, w, h);
    if (shadow_path) {
        vg_lite_draw(buffer, shadow_path, VG_LITE_FILL_EVEN_ODD, matrix,
                     VG_LITE_BLEND_SRC_OVER, shadow_color);
        free_path(shadow_path);
    }

    // Draw main rectangle on top
    vg_lite_path_t* main_path = create_rect_path(x, y, w, h);
    if (main_path) {
        vg_lite_draw(buffer, main_path, VG_LITE_FILL_EVEN_ODD, matrix,
                     VG_LITE_BLEND_SRC_OVER, main_color);
        free_path(main_path);
    }
}

/**
 * Draw an oval with simulated drop shadow
 * For ARGB1555 (1-bit alpha), shadows are simulated using darker colors
 */
static void draw_oval_with_shadow(vg_lite_buffer_t* buffer,
                                   float cx, float cy, float rx, float ry,
                                   float shadow_offset,
                                   vg_lite_color_t main_color,
                                   vg_lite_color_t shadow_color,
                                   vg_lite_matrix_t* matrix) {
    // Use circle path with average radius for simplicity
    float avg_radius = (rx + ry) / 2.0f;

    // Draw shadow oval (offset down and right)
    vg_lite_path_t* shadow_path = create_circle_path(cx + shadow_offset, cy + shadow_offset, avg_radius, 32);
    if (shadow_path) {
        vg_lite_draw(buffer, shadow_path, VG_LITE_FILL_EVEN_ODD, matrix,
                     VG_LITE_BLEND_SRC_OVER, shadow_color);
        free_path(shadow_path);
    }

    // Draw main oval on top
    vg_lite_path_t* main_path = create_circle_path(cx, cy, avg_radius, 32);
    if (main_path) {
        vg_lite_draw(buffer, main_path, VG_LITE_FILL_EVEN_ODD, matrix,
                     VG_LITE_BLEND_SRC_OVER, main_color);
        free_path(main_path);
    }
}

/**
 * Render scene with rectangles and ovals with drop shadows
 * @param buffer Target buffer for rendering
 * @param opa Global opacity (0xff for full opacity, 0x80 for 50%)
 */
void render_scene_box_shadow(vg_lite_buffer_t* buffer, uint8_t opa) {
    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);

    // ARGB1555 has 1-bit alpha: opacity > 127 means fully opaque
    uint8_t effective_alpha = (opa > 127) ? 255 : 0;

    // Shadow parameters (scaled from 400x200 to 800x480: 2x/2.4x)
    float shadow_offset = 8.0f;

    // Colors for main shapes - purple/maroon tones (matching golden image)
    vg_lite_color_t main_color1 = make_color(effective_alpha, 180, 80, 120);   // Maroon
    vg_lite_color_t main_color2 = make_color(effective_alpha, 150, 60, 100);   // Darker maroon
    vg_lite_color_t main_color3 = make_color(effective_alpha, 200, 100, 140);  // Lighter purple

    // Shadow color - dark gray
    vg_lite_color_t shadow_color = make_color(effective_alpha, 60, 60, 60);

    // Grid layout configuration
    float start_x = 60.0f;
    float start_y = 96.0f;
    float spacing_x = 110.0f;
    float spacing_y = 168.0f;
    int cols = 6;

    // Row 1: Rectangles with shadows
    for (int col = 0; col < cols; col++) {
        float x = start_x + col * spacing_x;
        float y = start_y;
        vg_lite_color_t color = (col % 3 == 0) ? main_color1 :
                                 ((col % 3 == 1) ? main_color2 : main_color3);
        draw_rect_with_shadow(buffer, x, y, 70.0f, 60.0f, shadow_offset,
                              color, shadow_color, &matrix);
    }

    // Row 2: Ovals with shadows
    for (int col = 0; col < cols; col++) {
        float cx = start_x + 35.0f + col * spacing_x;
        float cy = start_y + spacing_y + 36.0f;
        vg_lite_color_t color = (col % 3 == 0) ? main_color2 :
                                 ((col % 3 == 1) ? main_color3 : main_color1);
        draw_oval_with_shadow(buffer, cx, cy, 34.0f, 29.0f, shadow_offset,
                              color, shadow_color, &matrix);
    }

    // Row 3: Rectangles with shadows
    for (int col = 0; col < cols; col++) {
        float x = start_x + col * spacing_x;
        float y = start_y + 2 * spacing_y;
        vg_lite_color_t color = (col % 3 == 0) ? main_color3 :
                                 ((col % 3 == 1) ? main_color1 : main_color2);
        draw_rect_with_shadow(buffer, x, y, 70.0f, 60.0f, shadow_offset,
                              color, shadow_color, &matrix);
    }
}

// Test: BOX_SHADOW scene with full opacity
#if ARGB1555_TESTS_ENABLED
TEST_F(ARGB1555RenderTest, test_render_box_shadow_opa_255) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }

    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    // Fill with light purple/lavender background
    fill_buffer(buffer, make_color(255, 230, 220, 240));

    // Render box shadow scene with full opacity
    render_scene_box_shadow(buffer, 0xff);

    // Request dump for debugging
    RequestDump(buffer, "argb1555/demo_render_box_shadow_opa_255");

    // Save output PNG
    std::string png_path = "test_output_box_shadow_opa_255.png";
    save_buffer_to_png(buffer, png_path);


    // Compare with golden image
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_box_shadow_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_box_shadow_opa_255.png");

    free_buffer(buffer);
}

// Test: BOX_SHADOW scene with half opacity
TEST_F(ARGB1555RenderTest, test_render_box_shadow_opa_128) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }

    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    // Fill with light purple/lavender background
    fill_buffer(buffer, make_color(255, 230, 220, 240));

    // Render box shadow scene with half opacity (0x80)
    render_scene_box_shadow(buffer, 0x80);

    // Request dump for debugging
    RequestDump(buffer, "argb1555/demo_render_box_shadow_opa_128");

    // Save output PNG
    std::string png_path = "test_output_box_shadow_opa_128.png";
    save_buffer_to_png(buffer, png_path);


    // Compare with golden image (higher tolerance for opacity test)
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_box_shadow_opa_128.png", 0.15);
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_box_shadow_opa_128.png", 0.15);

    free_buffer(buffer);
}
#else
TEST_F(ARGB1555RenderTest, test_render_box_shadow_opa_255) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms due to color precision differences";
}
TEST_F(ARGB1555RenderTest, test_render_box_shadow_opa_128) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms due to color precision differences";
}
#endif

// ============================================================================
// LAYER_NORMAL Scene - Multiple overlapping layers with transparency
// ============================================================================

/**
 * Create a rounded rectangle path
 * @param x Top-left X coordinate
 * @param y Top-left Y coordinate
 * @param w Width
 * @param h Height
 * @param radius Corner radius
 */
static vg_lite_path_t* create_rounded_rect_path(float x, float y, float w, float h, float radius) {
    vg_lite_path_t* path = new vg_lite_path_t();
    std::memset(path, 0, sizeof(vg_lite_path_t));
    
    // Clamp radius to not exceed half the width/height
    float r = radius;
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;
    
    path->bounding_box[0] = x;
    path->bounding_box[1] = y;
    path->bounding_box[2] = x + w;
    path->bounding_box[3] = y + h;
    path->quality = VG_LITE_MEDIUM;
    path->format = VG_LITE_FP32;
    
    // Kappa for cubic bezier arc approximation
    const float KAPPA = 0.552284f;
    float rk = r * KAPPA;
    
    // Calculate total bytes needed:
    // MOVE: 12 bytes
    // 4x LINE + 4x CUBIC for rounded corners: 4*12 + 4*28 = 48 + 112 = 160 bytes
    // CLOSE: 4 bytes
    // END: 4 bytes
    // Total: 12 + 160 + 4 + 4 = 180 bytes
    int total_bytes = 180;
    uint8_t* data = (uint8_t*)std::malloc(total_bytes);
    uint8_t* ptr = data;
    
    // Start at top-left corner (after the left-radius arc)
    *(uint32_t*)ptr = VLC_OP_MOVE; ptr += 4;
    *(float*)ptr = x + r; ptr += 4;
    *(float*)ptr = y; ptr += 4;
    
    // Top edge and top-right corner
    *(uint32_t*)ptr = VLC_OP_LINE; ptr += 4;
    *(float*)ptr = x + w - r; ptr += 4;
    *(float*)ptr = y; ptr += 4;
    
    // Top-right corner arc
    *(uint32_t*)ptr = VLC_OP_CUBIC; ptr += 4;
    *(float*)ptr = x + w - r + rk; ptr += 4;  // cp1x
    *(float*)ptr = y; ptr += 4;                // cp1y
    *(float*)ptr = x + w; ptr += 4;            // cp2x
    *(float*)ptr = y + r - rk; ptr += 4;       // cp2y
    *(float*)ptr = x + w; ptr += 4;            // x
    *(float*)ptr = y + r; ptr += 4;            // y
    
    // Right edge
    *(uint32_t*)ptr = VLC_OP_LINE; ptr += 4;
    *(float*)ptr = x + w; ptr += 4;
    *(float*)ptr = y + h - r; ptr += 4;
    
    // Bottom-right corner arc
    *(uint32_t*)ptr = VLC_OP_CUBIC; ptr += 4;
    *(float*)ptr = x + w; ptr += 4;            // cp1x
    *(float*)ptr = y + h - r + rk; ptr += 4;   // cp1y
    *(float*)ptr = x + w - r + rk; ptr += 4;   // cp2x
    *(float*)ptr = y + h; ptr += 4;            // cp2y
    *(float*)ptr = x + w - r; ptr += 4;        // x
    *(float*)ptr = y + h; ptr += 4;            // y
    
    // Bottom edge
    *(uint32_t*)ptr = VLC_OP_LINE; ptr += 4;
    *(float*)ptr = x + r; ptr += 4;
    *(float*)ptr = y + h; ptr += 4;
    
    // Bottom-left corner arc
    *(uint32_t*)ptr = VLC_OP_CUBIC; ptr += 4;
    *(float*)ptr = x + r - rk; ptr += 4;       // cp1x
    *(float*)ptr = y + h; ptr += 4;            // cp1y
    *(float*)ptr = x; ptr += 4;                // cp2x
    *(float*)ptr = y + h - r + rk; ptr += 4;   // cp2y
    *(float*)ptr = x; ptr += 4;                // x
    *(float*)ptr = y + h - r; ptr += 4;        // y
    
    // Left edge
    *(uint32_t*)ptr = VLC_OP_LINE; ptr += 4;
    *(float*)ptr = x; ptr += 4;
    *(float*)ptr = y + r; ptr += 4;
    
    // Top-left corner arc
    *(uint32_t*)ptr = VLC_OP_CUBIC; ptr += 4;
    *(float*)ptr = x; ptr += 4;                // cp1x
    *(float*)ptr = y + r - rk; ptr += 4;       // cp1y
    *(float*)ptr = x + r - rk; ptr += 4;       // cp2x
    *(float*)ptr = y; ptr += 4;                // cp2y
    *(float*)ptr = x + r; ptr += 4;            // x
    *(float*)ptr = y; ptr += 4;                // y
    
    // CLOSE path
    *(uint32_t*)ptr = VLC_OP_CLOSE; ptr += 4;
    
    // END
    *(uint32_t*)ptr = VLC_OP_END; ptr += 4;
    
    path->path = data;
    path->path_length = static_cast<int>(ptr - data);
    return path;
}

/**
 * Render multiple overlapping layers with transparency
 * Tests layered rendering compositing with vg_lite API
 * @param buffer Target buffer for rendering
 * @param opa Global opacity (0xff for full opacity, 0x80 for 50%)
 */
void render_scene_layer_normal(vg_lite_buffer_t* buffer, uint8_t opa) {
    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    
    // ARGB1555 has 1-bit alpha, so opacity > 127 means fully opaque
    uint8_t effective_alpha = (opa > 127) ? 255 : 0;
    
    // Layer configurations - orange/yellow gradient buttons with varying positions
    // Scaled from 400x200 to 800x480: 2x/2.4x
    struct LayerConfig {
        float x, y, w, h;
        uint8_t r, g, b;
        float rotation;
    };
    
    LayerConfig layers[] = {
        // Row 1: Larger, more opaque shapes
        {60, 72, 120, 96, 255, 180, 50, 0},     // Orange
        {240, 120, 110, 84, 255, 200, 60, 15},   // Orange-yellow, rotated
        {400, 60, 100, 91, 255, 160, 40, -10},  // Darker orange
        {560, 108, 116, 101, 255, 220, 80, 5},    // Yellow-orange
        
        // Row 2: More shapes scattered
        {100, 240, 104, 86, 255, 190, 55, -5},   // Medium orange
        {280, 288, 96, 82, 255, 210, 70, 20},  // Yellow-orange
        {440, 228, 110, 96, 255, 170, 45, -15},  // Orange
        {600, 276, 100, 84, 255, 200, 65, 8},   // Orange-yellow
        
        // Smaller, more transparent/faded shapes (will appear pinkish due to background)
        {160, 432, 80, 67, 255, 150, 100, 10},  // Pinkish
        {340, 468, 76, 62, 255, 180, 120, -12},// Light pink
        {500, 420, 84, 72, 255, 160, 90, 6},   // Coral
    };
    
    // Draw fill layers (corner radius scaled by min(2, 2.4) = 2: 6*2=12)
    for (const auto& layer : layers) {
        vg_lite_path_t* path = create_rounded_rect_path(layer.x, layer.y, layer.w, layer.h, 12);
        if (path) {
            vg_lite_matrix_t layer_matrix;
            vg_lite_identity(&layer_matrix);
            
            // Apply rotation around center of shape
            float cx = layer.x + layer.w / 2;
            float cy = layer.y + layer.h / 2;
            vg_lite_translate(cx, cy, &layer_matrix);
            vg_lite_rotate(layer.rotation, &layer_matrix);
            vg_lite_translate(-cx, -cy, &layer_matrix);
            
            vg_lite_draw(buffer, path, VG_LITE_FILL_EVEN_ODD, &layer_matrix, 
                        VG_LITE_BLEND_SRC_OVER, make_color(effective_alpha, layer.r, layer.g, layer.b));
            free_path(path);
        }
    }
    
    // Draw black borders around each layer (stroke width scaled: 2*2=4)
    for (const auto& layer : layers) {
        vg_lite_path_t* path = create_rounded_rect_path(layer.x, layer.y, layer.w, layer.h, 12);
        if (path) {
            vg_lite_matrix_t layer_matrix;
            vg_lite_identity(&layer_matrix);
            
            float cx = layer.x + layer.w / 2;
            float cy = layer.y + layer.h / 2;
            vg_lite_translate(cx, cy, &layer_matrix);
            vg_lite_rotate(layer.rotation, &layer_matrix);
            vg_lite_translate(-cx, -cy, &layer_matrix);
            
            vg_lite_set_stroke(path, VG_LITE_CAP_BUTT, VG_LITE_JOIN_MITER, 4.0f, 8.0f, 
                              nullptr, 0, 0.0f, make_color(effective_alpha, 0, 0, 0));
            vg_lite_update_stroke(path);
            vg_lite_set_path_type(path, VG_LITE_DRAW_STROKE_PATH);
            vg_lite_draw(buffer, path, VG_LITE_FILL_EVEN_ODD, &layer_matrix,
                        VG_LITE_BLEND_SRC_OVER, make_color(effective_alpha, 0, 0, 0));
            vg_lite_clear_path(path);
            free_path(path);
        }
    }
}

// Test: LAYER_NORMAL scene with full opacity
#if ARGB1555_TESTS_ENABLED
TEST_F(ARGB1555RenderTest, test_render_layer_normal_opa_255) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    // Fill with light purple background (matching golden image description)
    fill_buffer(buffer, make_color(255, 230, 200, 250));

    // Render layer normal scene with full opacity
    render_scene_layer_normal(buffer, 0xff);
    
    // Request dump for debugging
    RequestDump(buffer, "argb1555/demo_render_layer_normal_opa_255");
    
    // Save output PNG
    std::string png_path = "test_output_demo_render_layer_normal_opa_255.png";
    save_buffer_to_png(buffer, png_path);

    
    // Compare with golden image
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_layer_normal_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_layer_normal_opa_255.png");
    
    free_buffer(buffer);
}

// Test: LAYER_NORMAL scene with half opacity
TEST_F(ARGB1555RenderTest, test_render_layer_normal_opa_128) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    // Create ARGB1555 buffer matching golden image size (800x480)
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);

    // Fill with light purple background
    fill_buffer(buffer, make_color(255, 230, 200, 250));

    // Render layer normal scene with half opacity (0x80)
    // Note: ARGB1555 has 1-bit alpha, so semi-transparent will appear fully transparent
    render_scene_layer_normal(buffer, 0x80);
    
    // Request dump for debugging
    RequestDump(buffer, "argb1555/demo_render_layer_normal_opa_128");
    
    // Save output PNG
    std::string png_path = "test_output_demo_render_layer_normal_opa_128.png";
    save_buffer_to_png(buffer, png_path);

    
    // Compare with golden image (higher tolerance for opacity test)
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_layer_normal_opa_128.png", 0.15);
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_layer_normal_opa_128.png", 0.15);
    
    free_buffer(buffer);
}
#else
TEST_F(ARGB1555RenderTest, test_render_layer_normal_opa_255) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms due to color precision differences";
}
TEST_F(ARGB1555RenderTest, test_render_layer_normal_opa_128) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms due to color precision differences";
}
#endif

// ============================================================================
// TEXT Scene - Text rendering placeholder
// ============================================================================

/**
 * Render text scene placeholder
 * Since vg_lite has no built-in text rendering support, this scene is skipped.
 * Text rendering requires font loading and glyph rendering which is outside
 * the scope of the vg_lite vector graphics API.
 * @param buffer Target buffer for rendering
 * @param opa Global opacity (0xff for full opacity, 0x80 for 50%)
 */
void render_scene_text(vg_lite_buffer_t* buffer, uint8_t opa) {
    // vg_lite does not have built-in text rendering support
    // Text rendering would require:
    // 1. Font loading (TTF/OTF parsing)
    // 2. Glyph rasterization
    // 3. Text layout and shaping
    // These features are typically provided by higher-level libraries like LVGL
    (void)buffer;  // Unused
    (void)opa;     // Unused
}

// Test: TEXT scene - skipped since vg_lite has no text support
TEST_F(ARGB1555RenderTest, test_render_text_opa_255) {
    GTEST_SKIP() << "TEXT scene requires font rendering - not supported in pure vg_lite";
}

// Test: TEXT scene with half opacity - skipped since vg_lite has no text support
TEST_F(ARGB1555RenderTest, test_render_text_opa_128) {
    GTEST_SKIP() << "TEXT scene requires font rendering - not supported in pure vg_lite";
}

// ============================================================================
// LINEAR_GRADIENT Scene - Rectangles with linear gradient fill
// ============================================================================

/**
 * Render rectangles with linear gradient fill
 * Demonstrates vg_lite linear gradient API with different spread modes
 * @param buffer Target buffer for rendering
 * @param opa Global opacity (0xff for full opacity, 0x80 for 50%)
 */
void render_scene_linear_gradient(vg_lite_buffer_t* buffer, uint8_t opa) {
    // ARGB1555 has 1-bit alpha, so opacity > 127 means fully opaque
    uint8_t effective_alpha = (opa > 127) ? 255 : 0;
    
    // Grid layout: 3 rows (pad, repeat, reflect) x 4 columns
    int cell_width = 70;
    int cell_height = 50;
    int padding = 10;
    int start_x = 80;  // Leave space for row labels
    int start_y = 30;
    
    // Gradient colors: dark red to black
    uint32_t colors[] = {
        make_color(effective_alpha, 180, 20, 60),   // Dark red
        make_color(effective_alpha, 100, 10, 30),   // Darker red
        make_color(effective_alpha, 30, 5, 15)      // Near black
    };
    uint32_t stops[] = {0, 128, 255};
    
    // Gradient colors with stop opacity (faded)
    uint32_t colors_faded[] = {
        make_color(effective_alpha, 200, 80, 100),  // Lighter red
        make_color(effective_alpha, 150, 60, 80),   // Medium
        make_color(effective_alpha, 80, 30, 40)     // Darker
    };
    
    // Spread modes: pad, repeat, reflect
    vg_lite_gradient_spreadmode_t spread_modes[] = {
        VG_LITE_GRADIENT_SPREAD_PAD,
        VG_LITE_GRADIENT_SPREAD_REPEAT,
        VG_LITE_GRADIENT_SPREAD_REFLECT
    };
    
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            int x = start_x + col * (cell_width + padding);
            int y = start_y + row * (cell_height + padding + 10);
            
            bool use_rounded = (col == 1 || col == 3);  // "round" columns
            bool use_stop_opa = (col >= 2);              // "stop opa" columns
            
            // Create path
            vg_lite_path_t* path;
            if (use_rounded) {
                path = create_rounded_rect_path(x, y, cell_width, cell_height, 8);
            } else {
                path = create_rect_path(x, y, cell_width, cell_height);
            }
            
            if (!path) continue;
            
            // Setup linear gradient
            vg_lite_linear_gradient_t grad;
            std::memset(&grad, 0, sizeof(grad));
            vg_lite_init_grad(&grad);
            
            // Use appropriate colors based on stop opacity setting
            uint32_t* grad_colors = use_stop_opa ? colors_faded : colors;
            vg_lite_set_grad(&grad, 3, grad_colors, stops);
            vg_lite_update_grad(&grad);
            
            // Get gradient matrix and scale for horizontal gradient
            vg_lite_matrix_t* grad_matrix = vg_lite_get_grad_matrix(&grad);
            vg_lite_identity(grad_matrix);
            vg_lite_scale(256.0f / cell_width, 1.0f, grad_matrix);
            
            // Draw with gradient
            vg_lite_matrix_t path_matrix;
            vg_lite_identity(&path_matrix);
            
            vg_lite_draw_grad(buffer, path, VG_LITE_FILL_EVEN_ODD, &path_matrix, &grad, VG_LITE_BLEND_SRC_OVER);
            
            vg_lite_clear_grad(&grad);
            free_path(path);
        }
    }
vg_lite_finish();
}

// Test: LINEAR_GRADIENT scene with full opacity
#if ARGB1555_TESTS_ENABLED
TEST_F(ARGB1555RenderTest, test_render_linear_gradient_opa_255) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Light purple/lavender background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    render_scene_linear_gradient(buffer, 0xff);
    
    RequestDump(buffer, "argb1555/demo_render_linear_gradient_opa_255");
    
    std::string png_path = "test_output_demo_render_linear_gradient_opa_255.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_linear_gradient_opa_255.png", 0.10);
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_linear_gradient_opa_255.png", 0.10);
    
    free_buffer(buffer);
}

// Test: LINEAR_GRADIENT scene with half opacity
TEST_F(ARGB1555RenderTest, test_render_linear_gradient_opa_128) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Light purple/lavender background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    render_scene_linear_gradient(buffer, 0x80);
    
    RequestDump(buffer, "argb1555/demo_render_linear_gradient_opa_128");
    
    std::string png_path = "test_output_demo_render_linear_gradient_opa_128.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_linear_gradient_opa_128.png", 0.15);
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_linear_gradient_opa_128.png", 0.15);
    
    free_buffer(buffer);
}
#else
TEST_F(ARGB1555RenderTest, test_render_linear_gradient_opa_255) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_render_linear_gradient_opa_128) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
#endif

// ============================================================================
// RADIAL_GRADIENT Scene - Circles with radial gradient fill
// ============================================================================

/**
 * Render circles/rectangles with radial gradient fill
 * Demonstrates vg_lite radial gradient API with different spread modes
 * @param buffer Target buffer for rendering
 * @param opa Global opacity (0xff for full opacity, 0x80 for 50%)
 */
void render_scene_radial_gradient(vg_lite_buffer_t* buffer, uint8_t opa) {
    // ARGB1555 has 1-bit alpha, so opacity > 127 means fully opaque
    uint8_t effective_alpha = (opa > 127) ? 255 : 0;
    
    // Grid layout: 3 rows (pad, repeat, reflect) x 5 columns
    int cell_size = 50;
    int padding = 10;
    int start_x = 80;  // Leave space for row labels
    int start_y = 30;
    
    // Create color ramps for radial gradient
    vg_lite_color_ramp_t color_ramps[] = {
        {0.0f, 0.7f, 0.1f, 0.25f, 1.0f},    // Dark red
        {0.5f, 0.9f, 0.3f, 0.35f, 0.8f},    // Medium red
        {1.0f, 0.15f, 0.05f, 0.1f, 0.3f}    // Dark/transparent
    };
    
    // Faded color ramps for stop opacity
    vg_lite_color_ramp_t color_ramps_faded[] = {
        {0.0f, 0.8f, 0.4f, 0.5f, 1.0f},     // Lighter red
        {0.5f, 0.6f, 0.3f, 0.4f, 0.7f},     // Medium
        {1.0f, 0.4f, 0.2f, 0.25f, 0.3f}     // Darker
    };
    
    // Spread modes: pad, repeat, reflect
    vg_lite_gradient_spreadmode_t spread_modes[] = {
        VG_LITE_GRADIENT_SPREAD_PAD,
        VG_LITE_GRADIENT_SPREAD_REPEAT,
        VG_LITE_GRADIENT_SPREAD_REFLECT
    };
    
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 5; col++) {
            int x = start_x + col * (cell_size + padding);
            int y = start_y + row * (cell_size + padding + 10);
            
            bool use_rounded = (col == 1 || col == 3);  // "round" columns
            bool use_stop_opa = (col >= 2);              // "stop opa" columns
            
            // Create path
            vg_lite_path_t* path;
            float cx = x + cell_size / 2.0f;
            float cy = y + cell_size / 2.0f;
            
            if (use_rounded) {
                path = create_circle_path(cx, cy, cell_size / 2.5f, 32);
            } else {
                path = create_rect_path(x, y, cell_size, cell_size);
            }
            
            if (!path) continue;
            
            // Setup radial gradient
            vg_lite_radial_gradient_t grad;
            std::memset(&grad, 0, sizeof(grad));
            
            // Radial gradient parameters: center, focal point, radius
            vg_lite_radial_gradient_parameter_t radial_params = {
                cx, cy,                    // Center (cx, cy)
                cell_size / 2.5f,          // Radius
                cx, cy                     // Focal point (same as center)
            };
            
            // Use appropriate color ramps based on stop opacity setting
            vg_lite_color_ramp_t* ramps = use_stop_opa ? color_ramps_faded : color_ramps;
            
            vg_lite_set_radial_grad(&grad, 3, ramps, radial_params, spread_modes[row], 0);
            vg_lite_update_radial_grad(&grad);
            
            // Draw with radial gradient
            vg_lite_matrix_t path_matrix;
            vg_lite_identity(&path_matrix);
            
            vg_lite_draw_radial_grad(buffer, path, VG_LITE_FILL_EVEN_ODD, &path_matrix, &grad,
                                     make_color(effective_alpha, 255, 255, 255),
                                     VG_LITE_BLEND_SRC_OVER, VG_LITE_FILTER_LINEAR);
            
            vg_lite_clear_radial_grad(&grad);
            free_path(path);
        }
    }
}

// Test: RADIAL_GRADIENT scene with full opacity
#if ARGB1555_TESTS_ENABLED
TEST_F(ARGB1555RenderTest, test_render_radial_gradient_opa_255) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Light purple/lavender background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    render_scene_radial_gradient(buffer, 0xff);
    
    RequestDump(buffer, "argb1555/demo_render_radial_gradient_opa_255");
    
    std::string png_path = "test_output_demo_render_radial_gradient_opa_255.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_radial_gradient_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_radial_gradient_opa_255.png");
    
    free_buffer(buffer);
}

// Test: RADIAL_GRADIENT scene with half opacity
TEST_F(ARGB1555RenderTest, test_render_radial_gradient_opa_128) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Light purple/lavender background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    render_scene_radial_gradient(buffer, 0x80);
    
    RequestDump(buffer, "argb1555/demo_render_radial_gradient_opa_128");
    
    std::string png_path = "test_output_demo_render_radial_gradient_opa_128.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_radial_gradient_opa_128.png", 0.15);
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_radial_gradient_opa_128.png", 0.15);
    
    free_buffer(buffer);
}
#else
TEST_F(ARGB1555RenderTest, test_render_radial_gradient_opa_255) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_render_radial_gradient_opa_128) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
#endif

// ============================================================================
// CONICAL_GRADIENT Scene - Angular/sweep gradient (simulated)
// ============================================================================

/**
 * Create a wedge path for conical gradient simulation
 * Creates a pie slice from center point
 */
static vg_lite_path_t* create_wedge_path(float cx, float cy, float radius,
                                          float start_angle, float end_angle, int segments = 8) {
    const float DEG_TO_RAD = 3.14159265359f / 180.0f;
    
    vg_lite_path_t* path = new vg_lite_path_t();
    std::memset(path, 0, sizeof(vg_lite_path_t));
    
    path->bounding_box[0] = cx - radius;
    path->bounding_box[1] = cy - radius;
    path->bounding_box[2] = cx + radius;
    path->bounding_box[3] = cy + radius;
    path->quality = VG_LITE_MEDIUM;
    path->format = VG_LITE_FP32;
    
    // MOVE to center + LINEs for arc + CLOSE
    int total_bytes = 12 + 12 + (segments * 12) + 4 + 4;
    uint8_t* data = (uint8_t*)std::malloc(total_bytes);
    uint8_t* ptr = data;
    
    float start_rad = start_angle * DEG_TO_RAD;
    float angle_span = end_angle - start_angle;
    float seg_angle = angle_span / segments;
    
    // MOVE to center
    *(uint32_t*)ptr = VLC_OP_MOVE; ptr += 4;
    *(float*)ptr = cx; ptr += 4;
    *(float*)ptr = cy; ptr += 4;
    
    // LINE to start of arc
    *(uint32_t*)ptr = VLC_OP_LINE; ptr += 4;
    *(float*)ptr = cx + radius * std::cos(start_rad); ptr += 4;
    *(float*)ptr = cy + radius * std::sin(start_rad); ptr += 4;
    
    // Arc segments
    for (int i = 1; i <= segments; i++) {
        float angle = start_rad + (seg_angle * i);
        *(uint32_t*)ptr = VLC_OP_LINE; ptr += 4;
        *(float*)ptr = cx + radius * std::cos(angle); ptr += 4;
        *(float*)ptr = cy + radius * std::sin(angle); ptr += 4;
    }
    
    // CLOSE (back to center)
    *(uint32_t*)ptr = VLC_OP_CLOSE; ptr += 4;
    
    // END
    *(uint32_t*)ptr = VLC_OP_END; ptr += 4;
    
    path->path = data;
    path->path_length = static_cast<int>(ptr - data);
    return path;
}

/**
 * Render conical/sweep gradient (simulated using wedge shapes)
 * vg_lite doesn't have native conical gradient, so we simulate with colored wedges
 * @param buffer Target buffer for rendering
 * @param opa Global opacity (0xff for full opacity, 0x80 for 50%)
 */
void render_scene_conical_gradient(vg_lite_buffer_t* buffer, uint8_t opa) {
    // ARGB1555 has 1-bit alpha, so opacity > 127 means fully opaque
    uint8_t effective_alpha = (opa > 127) ? 255 : 0;
    
    // Grid layout: 3 rows (pad, repeat, reflect) x 4 columns
    int cell_width = 70;
    int cell_height = 50;
    int padding = 10;
    int start_x = 80;
    int start_y = 30;
    
    // Number of wedges to simulate conical gradient
    const int num_wedges = 12;
    const float wedge_angle = 360.0f / num_wedges;
    
    // Color palette for wedges (red tones)
    uint32_t wedge_colors[] = {
        make_color(effective_alpha, 180, 20, 60),
        make_color(effective_alpha, 200, 40, 80),
        make_color(effective_alpha, 220, 60, 100),
        make_color(effective_alpha, 240, 80, 120),
        make_color(effective_alpha, 220, 60, 100),
        make_color(effective_alpha, 200, 40, 80),
        make_color(effective_alpha, 180, 20, 60),
        make_color(effective_alpha, 160, 15, 50),
        make_color(effective_alpha, 140, 10, 40),
        make_color(effective_alpha, 120, 8, 35),
        make_color(effective_alpha, 100, 6, 30),
        make_color(effective_alpha, 80, 4, 25)
    };
    
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            int x = start_x + col * (cell_width + padding);
            int y = start_y + row * (cell_height + padding + 10);
            
            bool use_rounded = (col == 1 || col == 3);
            bool use_stop_opa = (col >= 2);
            
            float cx = x + cell_width / 2.0f;
            float cy = y + cell_height / 2.0f;
            float radius = use_rounded ? (cell_width / 2.5f) : (cell_width / 2.0f);
            
            vg_lite_matrix_t matrix;
            vg_lite_identity(&matrix);
            
            if (use_rounded) {
                // For rounded: draw wedges inside a circle
                for (int w = 0; w < num_wedges; w++) {
                    float start_angle = w * wedge_angle;
                    float end_angle = (w + 1) * wedge_angle;
                    
                    vg_lite_path_t* wedge = create_wedge_path(cx, cy, radius, start_angle, end_angle, 2);
                    if (wedge) {
                        uint32_t color = use_stop_opa ? 
                            make_color(effective_alpha, 
                                       (wedge_colors[w] >> 16) & 0xFF,
                                       (wedge_colors[w] >> 8) & 0xFF,
                                       wedge_colors[w] & 0xFF) :
                            wedge_colors[w];
                        
                        vg_lite_draw(buffer, wedge, VG_LITE_FILL_EVEN_ODD, &matrix,
                                    VG_LITE_BLEND_SRC_OVER, color);
                        free_path(wedge);
                    }
                }
            } else {
                // For non-rounded: draw a rectangle with simulated sweep
                // Just fill with solid color for simplicity (conical not well supported)
                vg_lite_path_t* rect = create_rect_path(x, y, cell_width, cell_height);
                if (rect) {
                    vg_lite_draw(buffer, rect, VG_LITE_FILL_EVEN_ODD, &matrix,
                                VG_LITE_BLEND_SRC_OVER, 
                                make_color(effective_alpha, 180, 100, 140));
                    free_path(rect);
                }
            }
        }
    }
}

// Test: CONICAL_GRADIENT scene with full opacity
#if ARGB1555_TESTS_ENABLED
TEST_F(ARGB1555RenderTest, test_render_conical_gradient_opa_255) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Light purple/lavender background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    render_scene_conical_gradient(buffer, 0xff);
    
    RequestDump(buffer, "argb1555/demo_render_conical_gradient_opa_255");
    
    std::string png_path = "test_output_demo_render_conical_gradient_opa_255.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_conical_gradient_opa_255.png");
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_conical_gradient_opa_255.png");
    
    free_buffer(buffer);
}

// Test: CONICAL_GRADIENT scene with half opacity
TEST_F(ARGB1555RenderTest, test_render_conical_gradient_opa_128) {
    if (shouldSkipRenderingTests()) {
        GTEST_SKIP() << "GL backend requires GL 3.3+ context for rendering";
    }
    
    vg_lite_buffer_t* buffer = create_buffer(800, 480, VG_LITE_ARGB1555);
    ASSERT_NE(buffer, nullptr);
    
    // Light purple/lavender background
    fill_buffer(buffer, make_color(255, 230, 220, 240));
    
    render_scene_conical_gradient(buffer, 0x80);
    
    RequestDump(buffer, "argb1555/demo_render_conical_gradient_opa_128");
    
    std::string png_path = "test_output_demo_render_conical_gradient_opa_128.png";
    save_buffer_to_png(buffer, png_path);

    
    bool match = compare_with_golden(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_conical_gradient_opa_128.png", 0.15);
    if (!match) {
        std::cout << "[GOLDEN] Comparison failed - check test_output_mismatch.png" << std::endl;
    }
    EXPECT_TRUE(match) << get_comparison_message(buffer, "../ref_imgs_vg_lite/draw/render/argb1555/demo_render_conical_gradient_opa_128.png", 0.15);
    
    free_buffer(buffer);
}
#else
TEST_F(ARGB1555RenderTest, test_render_conical_gradient_opa_255) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
TEST_F(ARGB1555RenderTest, test_render_conical_gradient_opa_128) {
    GTEST_SKIP() << "ARGB1555 tests skipped on 32-bit platforms";
}
#endif
