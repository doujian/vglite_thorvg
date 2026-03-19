# ThorVG v1.0 API Migration Guide

This document outlines the key API changes required to migrate from ThorVG v0.15 to v1.0 in the `vg_lite_tvg.cpp` adapter layer.

## Overview of Breaking Changes

| Change | v0.15 API | v1.0 API |
|--------|-----------|----------|
| Stroke properties | `stroke(width)` | `strokeWidth(width)`, `strokeCap()`, `strokeJoin()` |
| Stroke color | `stroke(r,g,b,a)` | `strokeFill(r,g,b,a)` |
| Stroke dash | `stroke(pattern, count)` | `strokeDash(pattern, count, offset)` |
| Composite/Mask | `composite(paint, CompositeMethod::XXX)` | `mask(paint, MaskMethod::XXX)`, `clip(shape)` |
| Canvas Engine | `CanvasEngine::Sw`, `CanvasEngine::Gl` | Removed - just thread count |
| Fill Rule | `FillRule::Winding` | `FillRule::NonZero` |
| Blend Modes | Basic set | Extended with Hue, Saturation, Color, Luminosity, Add, Subtract, etc. |

---

## 1. Stroke API Changes (`stroke()` → Split Methods)

**Location:** `src/vg_lite_tvg.cpp` lines ~2465-2473

The v0.15 API used a single `stroke()` method to set multiple stroke properties. v1.0 splits this into dedicated methods.

### Before (v0.15):
```cpp
// Line 2465
TVG_CHECK_RETURN_RESULT(shape->stroke(path->stroke->line_width));

// Line 2467  
TVG_CHECK_RETURN_RESULT(shape->stroke(stroke_cap_conv(path->stroke->cap_style)));

// Line 2468
TVG_CHECK_RETURN_RESULT(shape->stroke(stroke_join_conv(path->stroke->join_style)));

// Line 2469
TVG_CHECK_RETURN_RESULT(shape->stroke(TVG_COLOR(path->stroke_color)));

// Line 2473
TVG_CHECK_RETURN_RESULT(shape->stroke(path->stroke->dash_pattern, path->stroke->pattern_count));
```

### After (v1.0):
```cpp
// Stroke width
TVG_CHECK_RETURN_RESULT(shape->strokeWidth(path->stroke->line_width));

// Stroke cap style
TVG_CHECK_RETURN_RESULT(shape->strokeCap(stroke_cap_conv(path->stroke->cap_style)));

// Stroke join style
TVG_CHECK_RETURN_RESULT(shape->strokeJoin(stroke_join_conv(path->stroke->join_style)));

// Stroke color
TVG_CHECK_RETURN_RESULT(shape->strokeFill(R(path->stroke_color), G(path->stroke_color), B(path->stroke_color), A(path->stroke_color)));

// Stroke dash pattern (note: v1.0 requires offset parameter)
TVG_CHECK_RETURN_RESULT(shape->strokeDash(path->stroke->dash_pattern, path->stroke->pattern_count, path->stroke->dash_phase));
```

---

## 2. CompositeMethod → MaskMethod + clip()

**Location:** `src/vg_lite_tvg.cpp`

The v0.15 `composite()` method is replaced by `mask()` and `clip()` in v1.0.

### Before (v0.15):
```cpp
// Line 694 - blit_rect
TVG_CHECK_RETURN_VG_ERROR(picture->composite(std::move(shape), CompositeMethod::ClipPath));

// Line 958 - mask operations  
TVG_CHECK_RETURN_VG_ERROR(picture->composite(std::move(mask), CompositeMethod::InvAlphaMask));

// Line 1232 - blit_rect another location
TVG_CHECK_RETURN_VG_ERROR(picture->composite(std::move(shape), CompositeMethod::ClipPath));

// Line 2565 - shape clip
TVG_CHECK_RETURN_RESULT(shape->composite(std::move(cilp), CompositeMethod::ClipPath));
```

### After (v1.0):

For clipping operations, use `clip()`:
```cpp
// Line 694 - Replace ClipPath with clip()
TVG_CHECK_RETURN_VG_ERROR(picture->clip(shape.release()));

// Line 1232 - Same replacement
TVG_CHECK_RETURN_VG_ERROR(picture->clip(shape.release()));

// Line 2565 - Same replacement for shapes
TVG_CHECK_RETURN_RESULT(shape->clip(cilp.release()));
```

For mask operations, use `mask()`:
```cpp
// Line 958 - Replace InvAlphaMask with mask()
TVG_CHECK_RETURN_VG_ERROR(picture->mask(mask.get(), MaskMethod::InvAlphaMask));
```

The new `MaskMethod` enum in v1.0 includes:
- `MaskMethod::None`
- `MaskMethod::Alpha`
- `MaskMethod::InvAlpha`
- `MaskMethod::Luma`
- `MaskMethod::InvLuma`
- `MaskMethod::Add` (NEW in v1.0)
- `MaskMethod::Subtract` (NEW in v1.0)
- `MaskMethod::Intersect` (NEW in v1.0)
- `MaskMethod::Difference` (NEW in v1.0)
- `MaskMethod::Lighten` (NEW in v1.0)
- `MaskMethod::Darken` (NEW in v1.0)

---

## 3. CanvasEngine Enum Removal

**Location:** `src/vg_lite_tvg.cpp` lines ~37-40, ~713, ~734

The `CanvasEngine` enum is removed in v1.0. Initialization now only requires thread count.

### Before (v0.15):
```cpp
// Lines 37-40 - Macro definition
#if VG_LITE_RENDER_BACKEND == VG_LITE_RENDER_GL
    #define TVG_CANVAS_ENGINE CanvasEngine::Gl
#else
    #define TVG_CANVAS_ENGINE CanvasEngine::Sw
#endif

// Line 713 - Initialization
TVG_CHECK_RETURN_VG_ERROR(Initializer::init(TVG_CANVAS_ENGINE, 0));

// Line 734 - Termination
TVG_CHECK_RETURN_VG_ERROR(Initializer::term(TVG_CANVAS_ENGINE));
```

### After (v1.0):

```cpp
// Remove the TVG_CANVAS_ENGINE macro - no longer needed

// Line 713 - Initialization (only thread count)
TVG_CHECK_RETURN_VG_ERROR(Initializer::init(0));

// Line 734 - Termination (no arguments)
TVG_CHECK_RETURN_VG_ERROR(Initializer::term());
```

The v1.0 `Initializer::init()` signature:
```cpp
static Result init(uint32_t threads = 0) noexcept;
```

The v1.0 `Initializer::term()` signature:
```cpp
static Result term() noexcept;
```

---

## 4. FillRule::Winding → FillRule::NonZero

**Location:** `src/vg_lite_tvg.cpp` line ~2291

The `FillRule::Winding` enum value is renamed to `FillRule::NonZero` in v1.0.

### Before (v0.15):
```cpp
// Line 2285-2291
static FillRule fill_rule_conv(vg_lite_fill_t fill)
{
    if(fill == VG_LITE_FILL_EVEN_ODD)
        return FillRule::EvenOdd;
    
    return FillRule::Winding;  // <-- This needs to change
}
```

### After (v1.0):
```cpp
static FillRule fill_rule_conv(vg_lite_fill_t fill)
{
    if(fill == VG_LITE_FILL_EVEN_ODD)
        return FillRule::EvenOdd;
    
    return FillRule::NonZero;  // Renamed from Winding
}
```

---

## 5. BlendMethod Additions

**Location:** `src/vg_lite_tvg.cpp` lines ~2294-2325

New blend modes are available in v1.0.

### Extended BlendMethod enum (v1.0):
```cpp
enum struct BlendMethod : uint8_t
{
    Normal = 0,
    Multiply,
    Screen,
    Overlay,
    Darken,
    Lighten,
    ColorDodge,
    ColorBurn,
    HardLight,
    SoftLight,
    Difference,
    Exclusion,
    Hue,               // NEW in v1.0
    Saturation,        // NEW in v1.0  
    Color,             // NEW in v1.0
    Luminosity,        // NEW in v1.0
    Add,               // NEW in v1.0
    Composition = 255  // NEW in v1.0
};
```

### Current adapter implementation (lines ~2294-2325):
```cpp
static BlendMethod blend_method_conv(vg_lite_blend_t blend)
{
    switch(blend) {
        case VG_LITE_BLEND_SRC_OVER:
            return BlendMethod::Normal;
        case VG_LITE_BLEND_DST_OVER:
            return BlendMethod::Normal;
        case VG_LITE_BLEND_SRC_ATOP:
            return BlendMethod::Normal;
        case VG_LITE_BLEND_SCREEN:
            return BlendMethod::Screen;
        case VG_LITE_BLEND_ADD:
            return BlendMethod::Add;
        case VG_LITE_BLEND_MULTIPLY:
            return BlendMethod::Multiply;
        case VG_LITE_BLEND_LIGHTEN:
            return BlendMethod::Lighten;
        case VG_LITE_BLEND_DARKEN:
            return BlendMethod::Darken;
    }
    return BlendMethod::Normal;
}
```

The adapter already handles `BlendMethod::Add` which was added in v0.15.

---

## Summary of Code Locations to Update

| Line(s) | Change Description |
|---------|---------------------|
| 37-40 | Remove/rewrite `TVG_CANVAS_ENGINE` macro |
| ~713 | Update `Initializer::init()` call |
| ~734 | Update `Initializer::term()` call |
| ~694 | Replace `CompositeMethod::ClipPath` with `clip()` |
| ~958 | Replace `CompositeMethod::InvAlphaMask` with `mask()` |
| ~1232 | Replace `CompositeMethod::ClipPath` with `clip()` |
| ~2291 | Change `FillRule::Winding` to `FillRule::NonZero` |
| ~2465 | Change `shape->stroke(width)` to `shape->strokeWidth(width)` |
| ~2467 | Change `shape->stroke(cap)` to `shape->strokeCap(cap)` |
| ~2468 | Change `shape->stroke(join)` to `shape->strokeJoin(join)` |
| ~2469 | Change `shape->stroke(color)` to `shape->strokeFill(r,g,b,a)` |
| ~2473 | Change `shape->stroke(pattern,cnt)` to `shape->strokeDash(pattern,cnt,offset)` |
| ~2565 | Replace `CompositeMethod::ClipPath` with `clip()` |

---

## Reference: ThorVG v1.0 Source Files

- Main header: `D:\Projects\thorvg-main\inc\thorvg.h`
- Common definitions: `D:\Projects\thorvg-main\src\common\tvgCommon.h`

### Key v1.0 APIs:
- `Shape::strokeWidth(float)` - Set stroke width
- `Shape::strokeCap(StrokeCap)` - Set stroke cap style
- `Shape::strokeJoin(StrokeJoin)` - Set stroke join style
- `Shape::strokeFill(uint8_t r, uint8_t g, uint8_t b, uint8_t a)` - Set stroke color
- `Shape::strokeFill(Fill*)` - Set stroke gradient
- `Shape::strokeDash(const float*, uint32_t, float)` - Set dash pattern
- `Paint::mask(Paint*, MaskMethod)` - Apply mask
- `Paint::clip(Shape*)` - Apply clip
- `Paint::blend(BlendMethod)` - Set blend mode
- `Initializer::init(uint32_t threads)` - Initialize engine
- `Initializer::term()` - Terminate engine