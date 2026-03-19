# VGLite + ThorVG 独立图形渲染库

## TL;DR

> **Quick Summary**: 基于 LVGL 的 vg_lite_tvg 参考实现，创建独立的 VGLite API 兼容库，内部使用 ThorVG C++ API 实现渲染功能。ThorVG 可配置使用软件渲染或 OpenGL 后端。
>
> **Deliverables**:
> - 独立的 VGLite 兼容库 (libvglite_tvg.a/.so/.dll)
> - VGLite API 头文件 (vg_lite.h)
> - C++ 适配层 (vg_lite_tvg.cpp) - VGLite → ThorVG 转换
> - C++ 矩阵操作 (vg_lite_matrix.cpp)
> - CMake 构建系统 (C++11 only)
> - 单元测试 + 示例程序
>
> **Estimated Effort**: Large
> **Parallel Execution**: YES - 4 waves
> **Critical Path**: Task 1 → Task 3 → Task 5 → Task 6 → Task 11 → Task 20 → F1-F4

---

## Context

### Original Request
从 LVGL 的 `src/debugging/vg_lite_tvg` 参考实现创建独立项目，VGLite API 兼容，内部使用 ThorVG 实现渲染。

### Correct Architecture (CRITICAL)

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Code                         │
│                  (uses vg_lite_* API)                        │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    vg_lite.h (C API)                         │
│              Public API - VGLite compatible                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│               vg_lite_tvg.cpp (C++ Adapter)                  │
│         VGLite API → ThorVG C++ API conversion              │
│         - vg_lite_init() → Initializer::init()              │
│         - vg_lite_draw() → Shape + Canvas                   │
│         - vg_lite_blit() → Picture + Canvas                 │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    ThorVG Library                            │
│         Render Engine - supports multiple backends          │
│         - CanvasEngine::Sw (software rendering)             │
│         - CanvasEngine::Gl (OpenGL rendering)               │
└─────────────────────────────────────────────────────────────┘
```

### Interview Summary
**Key Discussions**:
- 项目定位：VGLite API 兼容库，ThorVG 作为渲染引擎
- 架构模式：C API → C++ 适配层 → ThorVG C++ API
- 参考代码：`D:\Projects\lvgl-master\src\debugging\vg_lite_tvg\`
- ThorVG 后端：软件渲染（默认），可选 OpenGL 后端
- Windows：ThorVG 可配置使用 OpenGL 3.3
- Android：ThorVG 可配置使用 OpenGL ES 3.1
- 构建系统：CMake，支持增量编译
- ThorVG 集成：Git Submodule 方式
- API 策略：完全兼容原 vg_lite API
- 线程模型：单线程（与参考实现一致）
- 内存管理：标准 C/C++ 库
- 测试：单元测试 + 示例程序

**Research Findings**:
- 参考实现在 `src/debugging/vg_lite_tvg/` 目录
- 主要文件：`vg_lite_tvg.cpp`, `vg_lite_matrix.cpp`, `vg_lite.h`
- 使用 ThorVG C++ API：`Shape::gen()`, `Picture::gen()`, `SwCanvas::gen()`
- 当前使用 `CanvasEngine::Sw` 软件渲染

### Metis Review
**Identified Gaps** (addressed):
- 后端选择：ThorVG 内置支持 Sw/Gl 后端，通过 CanvasEngine 枚举选择
- 构建配置：ThorVG 的 OpenGL 后端需要特定 CMake 选项
- LVGL 依赖：需要移除所有 lv_* 符号引用

---

## Work Objectives

### Core Objective
创建一个 VGLite API 兼容的渲染库，内部使用 ThorVG 作为渲染引擎，支持软件和 OpenGL 两种渲染后端。

### Concrete Deliverables
- `D:\Projects\extract_vglite_by_thorvg/` 项目目录
- `include/vg_lite.h` — 公共 API 头文件（VGLite 兼容）
- `src/vg_lite_tvg.cpp` — C++ 适配层（核心实现，支持 SW/GL 后端宏切换）
- `src/vg_lite_matrix.cpp` — 矩阵操作（C++）
- `third_party/thorvg/` — ThorVG submodule
- `tests/unity/` — Unity 测试框架
- `tests/ref_imgs/` — Golden reference images（从 LVGL ref_imgs_vg_lite 复制）
  - `draw/` — 矢量绘图 golden
  - `widgets/` — VGLite 专用 widget golden
- `tests/assets/` — 测试资源文件（SVG、图片）
- `tests/test_draw_vector.c` — 矢量绘图测试（从 LVGL 移植）
- `tests/test_draw_vector_detail.c` — 矢量绘图详细测试（从 LVGL 移植）
- `examples/` — 示例程序
- `CMakeLists.txt` — CMake 构建系统（含 `VG_LITE_USE_GL_BACKEND` 选项）

### File Structure
```
D:\Projects\extract_vglite_by_thorvg\
├── include/
│   └── vg_lite.h           # 公共 API（从 vg_lite_tvg 复制）
├── src/
│   ├── vg_lite_tvg.cpp     # 核心 C++ 适配层（ThorVG 调用）
│   └── vg_lite_matrix.cpp  # 矩阵操作（C++）
├── third_party/
│   ├── CMakeLists.txt
│   └── thorvg/             # Git submodule
├── tests/
│   ├── CMakeLists.txt
│   ├── unity/              # Unity test framework
│   │   ├── unity.c
│   │   ├── unity.h
│   │   └── unity_internals.h
│   ├── test_helpers.h      # Test utilities
│   ├── test_config.h       # Test configuration
│   ├── test_main.c         # Basic init/close tests
│   ├── test_draw_vector.c  # Vector core tests (from LVGL)
│   ├── test_draw_vector_detail.c  # Vector detail tests
│   ├── test_svg_render.c   # SVG rendering tests (optional)
│   ├── test_canvas_buffer.c  # Canvas buffer tests
│   ├── test_color_formats.c  # Color format tests
│   ├── assets/             # Test input assets
│   │   ├── svg/            # SVG test files
│   │   └── images/         # Image test files
│   └── ref_imgs/           # Golden reference images (from ref_imgs_vg_lite)
│       ├── draw/           # Vector/SVG drawing golden
│       ├── libs/           # Library test golden
│       └── widgets/        # Widget golden
├── examples/
│   ├── CMakeLists.txt
│   └── hello_path.c
├── CMakeLists.txt
└── README.md
```

### Definition of Done
- [ ] `cmake -B build && cmake --build build` 成功（Windows MSVC）
- [ ] `ctest --test-dir build --output-on-failure` 全部通过
- [ ] vg_lite_init() → vg_lite_draw() → vg_lite_close() 流程正常
- [ ] 无 LVGL 依赖（grep -r "lv_" src/ 无匹配）
- [ ] 至少 10+ LVGL VGLite 测试用例成功移植并通过
- [ ] 测试覆盖率报告生成

### Must Have
- vg_lite.h API 100% 兼容
- CMake 构建支持 Windows
- ThorVG submodule
- C++ 适配层实现核心 API
- **双渲染后端支持**：SW（软件）和 GL（OpenGL）
- **宏切换后端**：`VG_LITE_RENDER_SW` / `VG_LITE_RENDER_GL`

### Backend Selection (CRITICAL)

使用编译时宏选择 ThorVG 渲染后端：

```cpp
// 默认：软件渲染
#define VG_LITE_RENDER_SW   1
#define VG_LITE_RENDER_GL   2

// 选择后端（默认 SW）
#ifndef VG_LITE_RENDER_BACKEND
#define VG_LITE_RENDER_BACKEND VG_LITE_RENDER_SW
#endif

// vg_lite_tvg.cpp 中：
#if VG_LITE_RENDER_BACKEND == VG_LITE_RENDER_GL
    #define TVG_CANVAS_ENGINE CanvasEngine::Gl
#else
    #define TVG_CANVAS_ENGINE CanvasEngine::Sw
#endif
```

**CMake 选项**：
```cmake
option(VG_LITE_USE_GL_BACKEND "Use ThorVG OpenGL backend" OFF)
if(VG_LITE_USE_GL_BACKEND)
    add_definitions(-DVG_LITE_RENDER_BACKEND=VG_LITE_RENDER_GL)
endif()
```

### Must NOT Have (Guardrails)
- NO LVGL 绘制层代码 (lv_draw_vg_lite*.c)
- NO lv_* 符号引用
- NO 窗口管理代码（GLFW/SDL）
- NO 硬件 VGLite GPU 支持（NXP 芯片）
- NO 新增公共 API（仅 vg_lite.h）
- NO 自己写 OpenGL 渲染代码（使用 ThorVG 内置后端）

---

## Verification Strategy (MANDATORY)

> **ZERO HUMAN INTERVENTION** — ALL verification is agent-executed.

### Test Decision
- **Infrastructure exists**: NO (new project)
- **Automated tests**: YES (TDD workflow)
- **Framework**: Google Test (C++) - modern C++ testing
- **TDD**: Each task follows RED → GREEN → REFACTOR

### QA Policy
Every task MUST include agent-executed QA scenarios.

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Start Immediately — scaffolding + extraction):
├── Task 1: Project scaffolding + CMake setup [quick]
├── Task 2: ThorVG submodule integration [quick]
├── Task 3: Copy and adapt vg_lite.h [quick]
├── Task 4: Copy and adapt vg_lite_matrix.c [quick]
└── Task 5: Copy and adapt vg_lite_tvg.cpp [deep]

Wave 2 (After Wave 1 — cleanup + testing):
├── Task 6: Remove LVGL dependencies [quick]
├── Task 7: Add build verification [quick]
├── Task 8: Create basic unit tests [quick]
└── Task 9: Create example program [quick]

Wave 3 (After Wave 2 — LVGL test case migration):
├── Task 10: Set up Unity test framework [quick]
├── Task 11: Port Vector Core Tests [deep]
├── Task 12: Port Vector Detail Tests [deep]
├── Task 13: Port SVG Parser Tests [quick] (optional)
├── Task 14: Port SVG Rendering Tests [deep]
├── Task 15: Port Canvas Integration Tests [quick]
├── Task 16: Port FreeType Outline Tests [quick] (optional)
├── Task 17: Port Render-to-Buffer Tests [unspecified-high]
├── Task 18: Create Test Assets [quick]
└── Task 19: Create Test Helper Utilities [quick]

Wave 4 (After Wave 3 — test execution + verification):
└── Task 20: Run and Fix All Ported Tests [deep]

Wave FINAL (After Wave 4 — verification):
├── Task F1: Plan compliance audit [oracle]
├── Task F2: Code quality review [unspecified-high]
├── Task F3: API compatibility test [unspecified-high]
└── Task F4: Test coverage report [unspecified-high]
-> Present results -> Get explicit user okay

Critical Path: Task 1 → Task 5 → Task 6 → Task 11 → Task 20 → F1-F4
Parallel Speedup: ~60% faster than sequential
Max Concurrent: 5 (Wave 1)
```

### Dependency Matrix

| Task | Depends On | Blocks |
|------|------------|--------|
| 1 | — | 2-9 |
| 2 | 1 | 5 |
| 3 | 1 | 4-5, 6-9 |
| 4 | 3 | 5, 6 |
| 5 | 2, 3, 4 | 6-9 |
| 6 | 5 | 7-9 |
| 7 | 6 | 10-20 |
| 8 | 6 | F1-F4 |
| 9 | 6 | F1-F4 |
| 10 | 7 | 11-19 |
| 11-17 | 10 | 18-20 |
| 18-19 | 10 | 20 |
| 20 | 11-19 | F1-F4 |

---

## TODOs

### Wave 1: Project Scaffolding + Core Extraction

- [ ] 1. Project scaffolding + CMake setup

  **What to do**:
  - Create directory structure: `include/`, `src/`, `tests/`, `examples/`, `third_party/`
  - Create root `CMakeLists.txt` with project definition (C++11)
  - Configure for mixed C/C++ compilation
  - Add Windows MSVC support (DLL export macros)
  - **Add backend selection option**:
    ```cmake
    # 渲染后端选择
    option(VG_LITE_USE_GL_BACKEND "Use ThorVG OpenGL backend (default: OFF, use SW)" OFF)
    
    if(VG_LITE_USE_GL_BACKEND)
        add_compile_definitions(VG_LITE_RENDER_BACKEND=2)  # VG_LITE_RENDER_GL
        message(STATUS "Using ThorVG OpenGL backend")
    else()
        add_compile_definitions(VG_LITE_RENDER_BACKEND=1)  # VG_LITE_RENDER_SW
        message(STATUS "Using ThorVG Software backend")
    endif()
    ```

  **Must NOT do**:
  - NO LVGL include paths
  - NO Python scripts for build

  **Parallelization**:
  - **Can Run In Parallel**: NO (foundation task)
  - **Blocks**: All other tasks

  **References**:
  - `D:\Projects\lvgl-master\src\debugging\vg_lite_tvg\` — Reference implementation

  **Acceptance Criteria**:
  - [ ] `cmake -B build` succeeds on Windows
  - [ ] Directory structure created correctly
  - [ ] `-DVG_LITE_USE_GL_BACKEND=ON` enables GL backend

- [ ] 2. ThorVG submodule integration

  **What to do**:
  - Add thorvg as git submodule at `third_party/thorvg`
  - Add CMake FetchContent fallback
  - Configure thorvg build options (enable Gl engine if needed)
  - Create `third_party/CMakeLists.txt`

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 3-4)
  - **Blocks**: Task 5

  **References**:
  - `D:\Projects\lvgl-master\src\libs\thorvg\` — ThorVG in LVGL
  - `https://github.com/thorvg/thorvg` — Official repo

  **Acceptance Criteria**:
  - [ ] `git submodule status` shows thorvg
  - [ ] thorvg compiles as part of project

- [ ] 3. Copy and adapt vg_lite.h

  **What to do**:
  - Copy `src/debugging/vg_lite_tvg/vg_lite.h` to `include/vg_lite.h`
  - Remove LVGL-specific includes (lv_conf_internal.h)
  - Add independent type definitions
  - Document API compatibility guarantee

  **Must NOT do**:
  - NO lv_* type references
  - NO breaking API changes

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 2, 4)
  - **Blocks**: Task 4-5

  **References**:
  - `D:\Projects\lvgl-master\src\debugging\vg_lite_tvg\vg_lite.h` — Source header

  **Acceptance Criteria**:
  - [ ] `include/vg_lite.h` exists
  - [ ] No `lv_` string in file
  - [ ] All original function signatures preserved

- [ ] 4. Copy and adapt vg_lite_matrix.cpp

  **What to do**:
  - Copy `src/debugging/vg_lite_tvg/vg_lite_matrix.c` to `src/vg_lite_matrix.cpp`
  - Remove LVGL includes
  - C++ code, no changes to logic

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 2-3)
  - **Blocks**: Task 5

  **References**:
  - `D:\Projects\lvgl-master\src\debugging\vg_lite_tvg\vg_lite_matrix.c` — Source file

  **Acceptance Criteria**:
  - [ ] `src/vg_lite_matrix.cpp` exists
  - [ ] No `lv_` string in file
  - [ ] Compiles with C++11

- [ ] 5. Copy and adapt vg_lite_tvg.cpp (CORE)

  **What to do**:
  - Copy `src/debugging/vg_lite_tvg/vg_lite_tvg.cpp` to `src/vg_lite_tvg.cpp`
  - Remove LVGL includes and LV_LOG macros
  - Replace lv_malloc/lv_free with standard malloc/free
  - Replace LV_ASSERT with standard assert
  - Keep ThorVG C++ API calls unchanged
  - Keep `vg_lite_ctx` class and all converters
  - **Add backend macro support**:
    ```cpp
    // 在文件顶部添加后端选择宏
    #if VG_LITE_RENDER_BACKEND == VG_LITE_RENDER_GL
      #define TVG_CANVAS_ENGINE CanvasEngine::Gl
    #else
      #define TVG_CANVAS_ENGINE CanvasEngine::Sw
    #endif
    ```
  - 替换原有的 `#define TVG_CANVAS_ENGINE CanvasEngine::Sw`

  **Recommended Agent Profile**:
  - **Category**: `deep`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on all Wave 1 tasks)
  - **Blocks**: Task 6-9

  **References**:
  - `D:\Projects\lvgl-master\src\debugging\vg_lite_tvg\vg_lite_tvg.cpp` — Source file (1500+ lines)

  **Acceptance Criteria**:
  - [ ] `src/vg_lite_tvg.cpp` exists
  - [ ] No `lv_` string in file
  - [ ] Compiles with C++11
  - [ ] Core functions: vg_lite_init, vg_lite_close, vg_lite_draw, vg_lite_blit

### Wave 2: Cleanup + Testing

- [ ] 6. Remove LVGL dependencies

  **What to do**:
  - Run `grep -r "lv_" src/ include/` to find all LVGL references
  - Replace lv_malloc/lv_free with malloc/free
  - Replace lv_memzero with memset
  - Replace LV_LOG with printf or remove
  - Replace LV_ASSERT with assert
  - Verify clean build

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 5)
  - **Blocks**: Task 7-9

  **Acceptance Criteria**:
  - [ ] `grep -r "lv_" src/ include/` returns empty
  - [ ] Project compiles without LVGL headers

- [ ] 7. Add build verification

  **What to do**:
  - Verify Windows MSVC build
  - Verify library links correctly
  - Check for missing symbols

  **Acceptance Criteria**:
  - [ ] `cmake --build build` succeeds
  - [ ] Library file generated

- [ ] 8. Create basic unit tests

  **What to do**:
  - Create `tests/test_main.c` with basic init/close test
  - Test matrix operations
  - Test basic path creation

  **Acceptance Criteria**:
  - [ ] Tests compile and run
  - [ ] Basic init/close passes

- [ ] 9. Create example program

  **What to do**:
  - Create `examples/hello_path.c`
  - Initialize library
  - Draw a simple rectangle
  - Save output (if possible)

  **Acceptance Criteria**:
  - [ ] Example compiles
  - [ ] Example runs without crash

---

### Wave 3: VGLite Test Case Migration (From LVGL Tests)

> 移植 LVGL 测试框架中的 VGLite 相关测试用例，适配到独立 VGLite 库。
> 参考：`D:\Projects\lvgl-master\tests\src\test_cases\`

- [ ] 10. Set up Google Test framework

  **What to do**:
  - Use FetchContent to download Google Test
  - Create `tests/CMakeLists.txt` with GTest integration
  - Create test runner infrastructure
  - Configure CMake to link against GTest::gtest and GTest::gtest_main

  **References**:
  - Google Test: https://github.com/google/googletest

  **Acceptance Criteria**:
  - [ ] Google Test framework compiles
  - [ ] Basic test runs

- [ ] 11. Port Vector Core Tests (test_draw_vector.cpp)

  **What to do**:
  - Copy core test functions from LVGL's `test_draw_vector.c`
  - Adapt to use standalone vg_lite_* API (remove lv_draw_vector_* wrapper)
  - Test coverage:
    - `test_draw_vector_render` — basic path rendering
    - `test_draw_vector_matrix` — matrix transforms (identity, translate, scale, rotate)
    - `test_draw_vector_linear_gradient` — linear gradient fill
    - `test_draw_vector_radial_gradient` — radial gradient fill
    - `test_draw_vector_image_fill` — image pattern fill
    - `test_draw_vector_blend_modes` — blend modes (src_over, screen, multiply, etc.)
    - `test_draw_vector_arc`, `test_draw_vector_circle`, `test_draw_vector_rect` — primitives
  - Remove LVGL-specific macros (TEST_DISPLAY_ROTATION_ASSERT_EQUAL_SCREENSHOT)
  - Add buffer comparison instead of screenshot comparison

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\draw\test_draw_vector.c` (448 lines, 5 tests)

  **Acceptance Criteria**:
  - [ ] `tests/test_draw_vector.c` exists
  - [ ] No `lv_` dependencies
  - [ ] Tests compile and run with Unity

- [ ] 12. Port Vector Detail Tests (test_draw_vector_detail.c)

  **What to do**:
  - Copy detailed path operation tests
  - Test coverage:
    - `test_draw_vector_blend` — all 15 blend modes
    - `test_draw_vector_path_copy` — path copying
    - `test_draw_vector_path_append_rect/arc/path` — path building
    - `test_draw_vector_path_composition` — path composition rules

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\draw\test_draw_vector_detail.c` (257 lines, 5 tests)

  **Acceptance Criteria**:
  - [ ] `tests/test_draw_vector_detail.c` exists
  - [ ] Tests pass for all blend modes

- [ ] 13. Port SVG Parser Tests (test_svg.c)

  **What to do**:
  - Copy SVG parser unit tests (no rendering, pure parsing)
  - Test coverage:
    - `test_svg_parse_basic` — element parsing
    - `test_svg_parse_path_data` — path commands (M, L, C, Q, Z, etc.)
    - `test_svg_parse_transform` — transform attributes
    - `test_svg_parse_fill_stroke` — style attributes
    - `test_svg_parse_gradient` — linear/radial gradients
    - `test_svg_parse_text` — text elements
  - Note: These test LVGL's SVG parser, not vg_lite directly, but useful for validation

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\test_svg.c` (14 tests)

  **Acceptance Criteria**:
  - [ ] `tests/test_svg_parser.c` exists (if SVG support is added)

- [ ] 14. Port SVG Rendering Tests (test_svg_file.c)

  **What to do**:
  - Copy SVG file rendering tests
  - Test coverage:
    - `test_svg_render_tiger` — tiger.svg rendering
    - `test_svg_render_gradients` — gradient tests
    - `test_svg_render_text` — text rendering
  - Replace `lv_draw_vector()` with direct vg_lite_draw() calls
  - Add test SVG assets to `tests/assets/`

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\test_svg_file.c` (1 test, 20+ SVG files)

  **Acceptance Criteria**:
  - [ ] `tests/test_svg_render.c` exists
  - [ ] Sample SVG files render correctly

- [ ] 15. Port Canvas Integration Tests (test_canvas.c)

  **What to do**:
  - Copy canvas buffer tests
  - Test coverage:
    - `test_canvas_buffer_create` — buffer allocation
    - `test_canvas_buffer_stride` — stride handling (VGLite specific)
    - `test_canvas_render_basic` — basic canvas rendering
  - Note: Canvas in LVGL uses lv_draw_* but we test the buffer operations

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\widgets\test_canvas.c` (16 tests)

  **Acceptance Criteria**:
  - [ ] `tests/test_canvas_buffer.c` exists
  - [ ] Buffer operations work correctly

- [ ] 16. Port FreeType Outline Tests (test_freetype.c)

  **What to do**:
  - Copy FreeType outline export tests
  - Test coverage:
    - `test_freetype_outline` — font to vg_lite_path conversion
    - `test_freetype_kerning` — kerning support
  - Only relevant if FreeType integration is added
  - Skip if FreeType not in scope

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\libs\test_freetype.c` (5 tests)

  **Acceptance Criteria**:
  - [ ] `tests/test_freetype_outline.c` exists (if FreeType enabled)

- [ ] 17. Port Render-to-Buffer Tests (test_render_to_*.c)

  **What to do**:
  - Copy render tests for different color formats
  - Test coverage:
    - ARGB8888, ARGB4444, ARGB1555, ARGB2222
    - RGB565, RGB565-swapped, RGB888
    - L8, AL88, I1 (indexed)
    - XRGB8888, ARGB8888-premultiplied
  - Adapt to use vg_lite_buffer_t directly
  - Test format conversion in vg_lite_tvg.cpp

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\draw\test_render_to_*.c` (12 files, 12 tests)

  **Acceptance Criteria**:
  - [ ] `tests/test_color_formats.c` exists
  - [ ] All supported formats render correctly

- [ ] 18. Create Test Assets and Golden Images

  **What to do**:
  - Copy test SVG files to `tests/assets/svg/`
  - Copy test images to `tests/assets/images/`
  - **Copy golden reference images from LVGL** (单一来源):
    - **全部从 `ref_imgs_vg_lite/` 复制**（这是 `LV_USE_DRAW_VG_LITE=1` 专门生成的 golden）
    - 目录结构：
      - `ref_imgs_vg_lite/draw/` — vector_draw_*.png + svg_*.png (100+ files)
      - `ref_imgs_vg_lite/libs/` — font_stress, svg_decoder, freetype, gif, etc. (88 files)
      - `ref_imgs_vg_lite/widgets/` — widget golden images (100+ files)
      - `ref_imgs_vg_lite/*.png` — 根目录文件 (flex, demo_vector_graphic, svg_1, etc.)

  **PowerShell 复制命令**:
  ```powershell
  # 创建目录
  New-Item -ItemType Directory -Force -Path "tests\ref_imgs\draw"
  New-Item -ItemType Directory -Force -Path "tests\ref_imgs\libs"
  New-Item -ItemType Directory -Force -Path "tests\ref_imgs\widgets"
  New-Item -ItemType Directory -Force -Path "tests\assets\svg"
  
  # 复制 VGLite golden images（全部从 ref_imgs_vg_lite）
  Copy-Item "D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\draw\*" "tests\ref_imgs\draw\" -Recurse -Force
  Copy-Item "D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\libs\*" "tests\ref_imgs\libs\" -Recurse -Force
  Copy-Item "D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\widgets\*" "tests\ref_imgs\widgets\" -Recurse -Force
  Copy-Item "D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\*.png" "tests\ref_imgs\" -Force
  
  # 复制 SVG 测试文件
  Copy-Item "D:\Projects\lvgl-master\tests\src\test_assets\svg\*" "tests\assets\svg\" -Recurse -Force
  
  # 重命名去除 .lp64/.lp32 后缀（选择 lp64 作为标准）
  Get-ChildItem "tests\ref_imgs" -Filter "*.lp64.png" -Recurse | ForEach-Object {
    Rename-Item $_.FullName ($_.Name -replace '\.lp64\.png$', '.png') -Force
  }
  # 删除 lp32 文件（可选，如果只用 lp64）
  Get-ChildItem "tests\ref_imgs" -Filter "*.lp32.png" -Recurse | Remove-Item -Force
  ```

  **Golden Data Source** (from LVGL):
  - **唯一来源**: `D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\` （VGLite 专用 golden）
  - 包含完整目录：draw/, libs/, widgets/, 根目录文件

  **References**:
  - `D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\` — VGLite 专用 golden 图片（完整）

  **Acceptance Criteria**:
  - [ ] Test assets directory populated
  - [ ] VGLite golden images copied from ref_imgs_vg_lite (draw/, libs/, widgets/)
  - [ ] Assets accessible from tests

- [ ] 19. Create Test Helper Utilities

  **What to do**:
  - Create `tests/test_helpers.h` with common utilities:
    - `test_buffer_create()` — allocate vg_lite_buffer_t
    - `test_buffer_compare()` — compare two buffers
    - `test_buffer_save_png()` — save buffer to PNG (optional)
    - `test_path_create_rect/circle()` — create test paths
  - Create `tests/test_config.h` with test configuration

  **Acceptance Criteria**:
  - [ ] Helper functions available
  - [ ] Tests use common utilities

### Wave 4: Test Execution + Verification

- [ ] 20. Run and Fix All Ported Tests

  **What to do**:
  - Run all ported tests
  - Fix compilation errors
  - Fix runtime failures
  - Document any skipped tests with reasons

  **Acceptance Criteria**:
  - [ ] All ported tests compile
  - [ ] Core vector tests pass
  - [ ] Known issues documented

---

## Test Case Summary (From LVGL)

### Direct VGLite Tests (High Priority)
| File | Tests | Status |
|------|-------|--------|
| test_draw_vector.c | 5 | Task 11 |
| test_draw_vector_detail.c | 5 | Task 12 |
| test_svg_file.c | 1 | Task 14 |
| test_canvas.c (stride) | ~5 | Task 15 |

### Format Tests (Medium Priority)
| File | Tests | Status |
|------|-------|--------|
| test_render_to_*.c | 12 | Task 17 |

### Optional Tests (Low Priority)
| File | Tests | Status |
|------|-------|--------|
| test_svg.c (parser) | 14 | Task 13 |
| test_freetype.c | 5 | Task 16 |

---

## Golden Data Strategy (测试参考数据)

### 关键发现：LVGL 有两套 Golden 图片

| 目录 | 用途 | 我们是否需要 |
|------|------|-------------|
| `lvgl/tests/ref_imgs/` | 通用参考（软件渲染等后端） | ❌ 不需要 |
| `lvgl/tests/ref_imgs_vg_lite/` | **VGLite 专用 golden** | ✅ **唯一来源** |

**重要**：`ref_imgs_vg_lite` 是专门为 `LV_USE_DRAW_VG_LITE=1` 配置生成的 golden 图片，与我们的项目直接对应。**该目录包含完整的 draw/, libs/, widgets/ 子目录，不需要从 ref_imgs/ 复制。**

### 目录结构

```
tests/
├── assets/                     # 输入测试资源
│   ├── svg/                    # SVG 测试文件
│   │   ├── tiger.svg
│   │   └── ...
│   └── images/                 # 图像测试文件
│       ├── test.png
│       └── ...
└── ref_imgs/                   # Golden reference images (expected output)
    ├── draw/                   # 矢量绘图 golden（从 ref_imgs_vg_lite/draw/ 复制）
    │   ├── vector_draw_shapes.lp64.png
    │   ├── vector_draw_lines.lp64.png
    │   ├── svg_file_tiger.lp64.png
    │   └── ... (100+ files)
    ├── libs/                   # 库测试 golden（从 ref_imgs_vg_lite/libs/ 复制）
    │   ├── font_stress/
    │   ├── svg_decoder_*.png
    │   ├── freetype_*.png
    │   └── ... (88 files)
    ├── widgets/                # Widget golden（从 ref_imgs_vg_lite/widgets/ 复制）
    │   ├── label_wrap_clip.png
    │   ├── slider_*.png
    │   └── ... (100+ files)
    └── *.png                   # 根目录 golden（flex, demo_vector_graphic, svg_1, etc.）
```

### Golden 图片来源（唯一来源：ref_imgs_vg_lite/）

**从 `lvgl/tests/ref_imgs_vg_lite/` 复制：**
```
draw/                      # 矢量/SVG 绘制测试
├── vector_draw_*.lp64.png    # 矢量绘图测试
├── svg_draw_*.lp64.png       # SVG 绘制测试
└── svg_file_*.lp64.png       # SVG 文件渲染测试

libs/                      # 库测试
├── font_stress/              # 字体压力测试
├── svg_decoder_*.png         # SVG 解码器
├── freetype_*.png            # FreeType 渲染
└── gif_*, jpg_*, png_*, etc. # 图像解码器

widgets/                   # Widget 测试 golden
├── label_*.png               # ~15 files
├── slider_*.png              # ~10 files
├── span_*.png                # ~17 files
├── spinner_*.png             # ~10 files
└── ... (总计 100+ files)

根目录/                     # 其他 golden
├── flex_*.png, grid_*.png    # 布局测试
├── demo_vector_graphic_*.png # 矢量演示
├── svg_*.png                 # SVG 测试
└── theme_*.png               # 主题测试
```

### 复制命令（PowerShell）

```powershell
# 创建目录
New-Item -ItemType Directory -Force -Path "tests\ref_imgs\draw"
New-Item -ItemType Directory -Force -Path "tests\ref_imgs\libs"
New-Item -ItemType Directory -Force -Path "tests\ref_imgs\widgets"
New-Item -ItemType Directory -Force -Path "tests\assets\svg"
New-Item -ItemType Directory -Force -Path "tests\assets\images"

# 全部从 ref_imgs_vg_lite 复制（唯一来源）
Copy-Item "D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\draw\*" "tests\ref_imgs\draw\" -Recurse -Force
Copy-Item "D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\libs\*" "tests\ref_imgs\libs\" -Recurse -Force
Copy-Item "D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\widgets\*" "tests\ref_imgs\widgets\" -Recurse -Force
Copy-Item "D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\*.png" "tests\ref_imgs\" -Force

# 复制 SVG 测试文件
Copy-Item "D:\Projects\lvgl-master\tests\src\test_assets\svg\*" "tests\assets\svg\" -Recurse -Force
```

### LP32 vs LP64 说明

LVGL golden 图片有两个版本：
- `.lp32.png` - 32位系统 (ILP32 数据模型)
- `.lp64.png` - 64位系统 (LP64 数据模型)

**我们的策略**：
- 只复制 `.lp64.png` 版本（Windows/现代系统都是 64 位）
- 重命名去除 `.lp64` 后缀：`vector_draw_shapes.lp64.png` → `vector_draw_shapes.png`

```powershell
# 重命名去除 .lp64 后缀
Get-ChildItem "tests\ref_imgs" -Filter "*.lp64.png" -Recurse | ForEach-Object {
    Rename-Item $_.FullName ($_.Name -replace '\.lp64\.png$', '.png')
}
```

### 图片对比方法

两种方案可选：

**方案 A：PNG 截图对比**（与 LVGL 一致）
```c
// 测试代码
test_buffer_save_png(buffer, "output.png");
test_assert_image_compare("output.png", "ref_imgs/widgets/label_1.png");
```

**方案 B：原始缓冲区对比**（更快，无文件 I/O）
```c
// 测试代码
test_assert_buffer_compare(rendered_buffer, expected_buffer, width, height);
```

**推荐**：方案 B 用于快速单元测试，方案 A 用于调试和生成 golden。

### Golden 文件统计

| 来源目录 | 文件类别 | 预估文件数 |
|----------|----------|-----------|
| `ref_imgs_vg_lite/draw/vector_*` | 矢量绘图 | ~40+ |
| `ref_imgs_vg_lite/draw/svg_*` | SVG 渲染 | ~60+ |
| `ref_imgs_vg_lite/libs/` | 库测试 (freetype, gif, jpg, etc.) | ~88 |
| `ref_imgs_vg_lite/widgets/` | Widget golden | ~100+ |
| `ref_imgs_vg_lite/*.png` | 根目录 (flex, demo, theme, etc.) | ~60 |
| **总计** | | **~350+** |

---

## Final Verification Wave (MANDATORY)

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Verify all "Must Have" implemented, all "Must NOT Have" absent.

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Run build, check for warnings/errors.

- [ ] F3. **API Compatibility Test** — `unspecified-high`
  Verify API matches reference vg_lite.h.

- [ ] F4. **Test Coverage Report** — `unspecified-high`
  Report test coverage from ported LVGL tests.

---

## Test Source Files (From LVGL)

### Primary VGLite Test Sources
以下 LVGL 测试文件包含直接测试 VGLite API 的代码，应作为移植优先级：

| 源文件 | 行数 | 测试数 | 主要覆盖 |
|--------|------|--------|----------|
| `tests/src/test_cases/draw/test_draw_vector.c` | 448 | 5 | 矩阵变换、渐变填充、图像填充、混合模式 |
| `tests/src/test_cases/draw/test_draw_vector_detail.c` | 257 | 5 | 路径操作、blend modes、composition |
| `tests/src/test_cases/test_demo_vector_graphic.c` | ~100 | 1 | 端到端矢量绘图演示 |
| `tests/src/test_cases/test_svg_file.c` | ~200 | 1 | SVG 文件渲染 (tiger.svg 等) |

### Secondary Test Sources
以下测试间接使用 VGLite，可选择性移植：

| 源文件 | 行数 | 测试数 | 主要覆盖 |
|--------|------|--------|----------|
| `tests/src/test_cases/draw/test_draw_svg.c` | ~300 | 6 | SVG 渲染 (group, shape, gradient, text) |
| `tests/src/test_cases/test_svg.c` | ~500 | 14 | SVG 解析器单元测试 |
| `tests/src/test_cases/test_svg_anim.c` | ~200 | 5 | SVG 动画解析 |
| `tests/src/test_cases/libs/test_svg_decoder.c` | ~100 | 3 | SVG 图像解码器 |
| `tests/src/test_cases/widgets/test_canvas.c` | ~400 | 16 | Canvas 缓冲区操作 |
| `tests/src/test_cases/libs/test_freetype.c` | ~800 | 5 | FreeType 轮廓导出 |
| `tests/src/test_cases/widgets/test_arclabel.c` | ~200 | 4 | 弧形标签渲染 |

### Render-to-Buffer Tests (Color Format Coverage)
| 源文件 | 覆盖格式 |
|--------|----------|
| `test_render_to_argb8888.c` | ARGB8888 |
| `test_render_to_argb4444.c` | ARGB4444 |
| `test_render_to_rgb565.c` | RGB565 |
| `test_render_to_rgb565_swap.c` | RGB565 (byte swap) |
| `test_render_to_xrgb8888.c` | XRGB8888 |
| `test_render_to_l8.c` | L8 (grayscale) |
| `test_render_to_a8.c` | A8 (alpha only) |
| `test_render_to_i1.c` | 1-bit indexed |
| ... (12 files total) | |

### Test Dependencies
这些测试依赖以下 LVGL 模块：
- `lv_draw_vector.h` — 矢量绘制 API
- `lv_vector_path.h` — 路径操作
- `lv_matrix.h` — 矩阵变换
- `lv_svg_*` — SVG 解析和渲染

**移植策略**：将 `lv_draw_vector_*` 调用替换为直接 `vg_lite_*` 调用。

---

## Commit Strategy

- **Wave 1**: `feat(init): project scaffolding and core extraction`
- **Wave 2**: `test: add unit tests and examples`
- **Wave 3**: `test: port LVGL VGLite test cases`
- **Wave 4**: `fix: resolve test failures and verify coverage`

---

## Success Criteria

### Verification Commands
```bash
# Windows build
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release

# Check no LVGL dependencies
grep -r "lv_" src/ include/  # Should return no matches

# Run tests
ctest --test-dir build -C Release --output-on-failure
```

### Final Checklist
- [ ] All "Must Have" present
- [ ] All "Must NOT Have" absent
- [ ] All tests pass on Windows
- [ ] No LVGL dependencies
- [ ] vg_lite API compatible
- [ ] Vector core tests (test_draw_vector.c) ported and passing
- [ ] Vector detail tests (test_draw_vector_detail.c) ported and passing
- [ ] SVG render tests (test_svg_file.c) ported and passing (optional)
- [ ] Canvas buffer tests (test_canvas.c) ported and passing
- [ ] Color format tests (render_to_*.c) ported and passing (optional)
- [ ] Test coverage report generated
