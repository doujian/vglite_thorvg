# Golden Image Manifest

This document maps each golden image to its corresponding test case.

## Summary

- **Total Golden Images**: 38
- **Unique Test Cases**: 8
- **Each image has `.lp64.png` and `.lp32.png` variants**

## Golden Image Mapping

| Golden Image | Test File | Test Function | Description |
|--------------|-----------|---------------|-------------|
| `vector_draw_shapes.lp{32,64}.png` | `test_draw_vector.cpp` | `VectorDrawTest.DrawShapes` | LVGL red/blue rectangles, green semi-transparent circle, radial gradient |
| `vector_draw_lines.lp{32,64}.png` | `test_draw_vector.cpp` | `VectorDrawTest.DrawLines` | Various line strokes with different stroke properties |
| `vector_draw_copy_path.lp{32,64}.png` | `test_draw_vector_detail.cpp` | `VectorDetailTest.DrawCopyPath` | Path copy test - creates path, copies it, draws both (blue/red triangles) |
| `vector_draw_rect_path.lp{32,64}.png` | `test_draw_vector_detail.cpp` | `VectorDetailTest.DrawRectPath` | Blue rectangle at (50,50) to (100,200) |
| `vector_draw_append_path.lp{32,64}.png` | `test_draw_vector_detail.cpp` | `VectorDetailTest.DrawAppendPath` | Two paths appended together with fill and stroke |
| `vector_draw_arc_path.lp{32,64}.png` | `test_draw_vector_detail.cpp` | `VectorDetailTest.DrawArcPath` | Arc stroke (red, 5px) and circle fill (blue) |
| `vector_draw_blend_modes_comparison.lp{32,64}.png` | `test_draw_vector_detail.cpp` | `VectorDetailTest.DISABLED_DrawBlendModesComparison` | 9 blend mode examples in 4x4 grid (DISABLED - requires full LVGL implementation) |

## Tests Without Golden Images

| Test File | Test Function | Status |
|-----------|---------------|--------|
| `test_draw_vector.cpp` | `VectorDrawTest.Transform` | Unit test only - validates matrix math via assertions |
| `test_golden.cpp` | `GoldenImageTest.FillRulesComparison` | No golden image - verifies rendering doesn't crash |
| `test_golden.cpp` | `GoldenImageTest.MatrixTransformComparison` | No golden image - verifies rendering doesn't crash |

## During Rendering Tests

These golden images include rotation variants for display rotation testing:

| Base Name | Rotation Variants |
|-----------|-------------------|
| `vector_draw_shapes` | `_disp_rotation_0`, `_disp_rotation_90`, `_disp_rotation_180`, `_disp_rotation_270` |
| `vector_draw_lines` | `_disp_rotation_0`, `_disp_rotation_90`, `_disp_rotation_180`, `_disp_rotation_270` |

## Notes

- All golden images are 800x480 resolution (matching LVGL test resolution)
- `.lp64` and `.lp32` variants correspond to 64-bit and 32-bit builds
- Golden images are located in `tests/ref_imgs/draw/`
- Test files are located in `tests/test_*.cpp`
