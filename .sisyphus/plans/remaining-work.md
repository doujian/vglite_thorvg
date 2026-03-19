# VGLite 剩余工作计划：测试迁移 + Android 适配

## TL;DR

> **Quick Summary**: 完成剩余的 Wave 3-4 测试用例迁移，并新增 Android 平台适配（OpenGL ES 后端 + 示例应用）。
> 
> **Deliverables**:
> - 修复硬编码路径问题
> - 完成 SVG/Canvas/颜色格式测试迁移
> - 启用 ThorVG GL 后端
> - Android NDK 构建支持
> - JNI 封装层
> - Android OpenGL ES 示例应用
> 
> **Estimated Effort**: Large
> **Parallel Execution**: YES - 5 waves
> **Critical Path**: Task 1 → Task 4 → Task 7 → Task 11 → Task 15 → F1-F4

---

## Context

### Original Request
完成原计划中剩余的 Wave 3-4 测试任务，并新增 Android 平台适配支持。

### 🔴 CRITICAL DISCOVERY (from Metis Analysis)

**ThorVG GL 后端未被编译到当前项目中！**

| 组件 | 当前状态 | Android 需求 |
|------|---------|-------------|
| `THORVG_GL_RASTER_SUPPORT` | 在 `config.h` 中**未定义** | 必须定义 |
| `tvgGlCanvas.cpp` | 存在但为**空壳**（条件编译） | 需要 GL 渲染器 |
| GL 渲染器源文件 | **不存在** | 需要 `tvgGlRenderer.cpp`, `tvgGlShader.cpp` 等 |

**重要**: 当前 `tvgGlCanvas.cpp` 只是一个空壳，当 `THORVG_GL_RASTER_SUPPORT` 未定义时返回 `NonSupport`。需要从 ThorVG 官方仓库获取完整的 GL 引擎源文件。

**解决方案选项**:
1. **选项 A (推荐)**: 使用 SW 后端作为 Android demo（更快，风险更低，当前可用）
2. **选项 B**: 从 ThorVG 官方仓库获取 GL 引擎源文件并启用

**本计划采用选项 B**：按用户要求启用 GL 后端，需要额外获取 GL 源文件。

### Interview Summary
**Key Discussions**:
- Wave 3-4: 继续完成全部剩余测试任务
- Android 适配: 需要 OpenGL ES 后端 + Android 示例应用
- GL 后端: 当前项目未包含 GL 渲染器，需要新增启用任务

**Research Findings**:
- ThorVG GL 后端源文件存在于 `third_party/thorvg/` 但未编译
- Android 需要 OpenGL ES 3.1
- 推荐 `android_native_app_glue` 模式而非 NativeActivity
- 当前 golden images 只有 76 个文件（draw/ 目录）

### Metis Review
**Identified Gaps** (addressed):
- **硬编码路径**: 测试文件中硬编码了 `D:/Projects/...` 路径，需修复
- **GL 后端缺失**: 需要先启用 ThorVG GL 后端才能支持 Android OpenGL ES
- **Golden Images**: `libs/` 和 `widgets/` 目录不存在，只使用现有 76 个 images
- **可选任务**: Task 13 (SVG Parser) 和 Task 16 (FreeType) 延后

---

## Work Objectives

### Core Objective
1. 完成剩余测试用例迁移（SVG/Canvas/颜色格式）
2. 启用 ThorVG GL 后端
3. 创建 Android 平台支持（NDK 构建 + JNI + 示例应用）

### Concrete Deliverables
- 修复硬编码路径的测试文件
- `tests/test_svg_render.cpp` — SVG 渲染测试
- `tests/test_canvas_buffer.cpp` — Canvas 缓冲区测试
- `tests/test_color_formats.cpp` — 颜色格式测试
- `third_party/CMakeLists.txt` 更新 — 包含 GL 后端源文件
- `src/vg_lite_tvg.cpp` 更新 — GL 后端宏支持
- `android/` — Android 项目目录
- `android/jni/vglite_jni.cpp` — JNI 封装层
- `android/app/src/main/` — Android 示例应用

### Definition of Done
- [ ] 所有硬编码路径修复为相对路径
- [ ] `ctest --test-dir build --output-on-failure` 全部通过
- [ ] ThorVG GL 后端成功编译
- [ ] Android NDK 构建成功 (`libvglite.so`)
- [ ] Android 示例应用在模拟器/设备上运行

### Must Have
- 修复硬编码路径（阻塞其他所有工作）
- **完整移植 LVGL VGLite 测试用例**（必须检查完整性）
- SVG 渲染测试
- Canvas 缓冲区测试
- 颜色格式测试
- **ThorVG GL 后端启用**（必须实现，如需自行适配）
- Android NDK 构建
- JNI 封装层
- Android OpenGL ES 示例应用
- **Golden 比较必须有效**（与 LVGL 测试结果判断一致）

### Must NOT Have (Guardrails)
- NO Task 13 (SVG Parser Tests) — 延后到后续迭代
- NO Task 16 (FreeType Tests) — 延后到后续迭代
- NO 创建新的 golden images — 只使用现有 76 个
- NO 引用 `libs/` 和 `widgets/` 目录 — 不存在
- NO Java/Kotlin UI 层 — 仅原生 C++ demo
- NO GameActivity — 使用 `android_native_app_glue`
- ~~NO 修复 `DrawBlendModesComparison`~~ — **低优先级修复**

### ThorVG 源文件来源 (CRITICAL)
1. **首选**: `D:\Projects\lvgl-master\src\libs\thorvg\` — LVGL 内置版本
2. **备用**: `D:\Projects\opencode_plan_test\thorvg-main\thorvg-main\` — 完整 ThorVG
3. **禁止**: GitHub 在线拉取（可能失败）

---

## Verification Strategy (MANDATORY)

> **ZERO HUMAN INTERVENTION** — ALL verification is agent-executed.

### Test Decision
- **Infrastructure exists**: YES (Google Test)
- **Automated tests**: YES (TDD workflow)
- **Framework**: Google Test (C++)

### QA Policy
Every task MUST include agent-executed QA scenarios.
Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 0 (CRITICAL - Must Complete First — path fix):
└── Task 1: Fix hardcoded paths in test files [quick]

Wave 3 (After Wave 0 — remaining test migration):
├── Task 2: Port SVG Rendering Tests [deep]
├── Task 3: Port Canvas Integration Tests [quick]
├── Task 4: Port Color Format Tests [unspecified-high]
└── Task 5: Verify and Document Golden Images [quick]

Wave 4 (After Wave 3 — test execution):
├── Task 6: Verify LVGL VGLite Test Case Completeness [deep]
├── Task 7: Run and Fix All Ported Tests [deep]
└── Task 8: Generate Coverage Report [quick]

Wave 5a (After Wave 4 — ThorVG GL backend enablement):
├── Task 9: Fetch ThorVG GL engine source files [quick]
├── Task 10: Update ThorVG config for GL backend [quick]
├── Task 11: Update CMakeLists.txt for GL sources [quick]
├── Task 12: Add GL backend macro support [quick]
└── Task 13: Test GL backend on Windows [deep]

Wave 5b (After Wave 5a — Android adaptation):
├── Task 14: Create Android project structure [quick]
├── Task 15: Create Android NDK CMakeLists.txt [quick]
├── Task 16: Create JNI wrapper layer [unspecified-high]
├── Task 17: Create Android demo app (native) [visual-engineering]
└── Task 18: Build and test on Android [deep]

Wave FINAL (After ALL tasks — verification):
├── Task F1: Plan compliance audit [oracle]
├── Task F2: Code quality review [unspecified-high]
├── Task F3: Real manual QA [unspecified-high]
└── Task F4: Test coverage report [unspecified-high]
-> Present results -> Get explicit user okay

Critical Path: Task 1 → Task 6 → Task 7 → Task 13 → Task 17 → F1-F4
Parallel Speedup: ~60% faster than sequential
Max Concurrent: 4 (Wave 3, 5a)
```

### Dependency Matrix

| Task | Depends On | Blocks |
|------|------------|--------|
| 1 | — | 2-18 |
| 2-5 | 1 | 6 |
| 6 | 2-5 | 7 |
| 7 | 6 | 8 |
| 8 | 7 | 9-18 |
| 9-12 | 8 | 13 |
| 13 | 9-12 | 14-18 |
| 14-15 | 13 | 16 |
| 16 | 14-15 | 17 |
| 17 | 16 | 18 |
| 18 | 17 | F1-F4 |
| F1-F4 | 18 | — |

---

## TODOs

### Wave 0: Fix Hardcoded Paths (CRITICAL - First)

- [ ] 1. Fix hardcoded paths in test files

  **What to do**:
  - 扫描所有测试文件中的硬编码路径 `D:/Projects/extract_vglite_by_thorvg/`
  - 替换为相对路径或使用 `CMAKE_CURRENT_SOURCE_DIR`
  - 更新 `test_helpers.h` 中的路径获取函数
  - 更新所有 `test_*.cpp` 文件中的 golden image 路径

  **Must NOT do**:
  - NO 创建新的路径解析逻辑 — 使用 CMake 提供的标准方法
  - NO 修改非测试文件的路径

  **Parallelization**:
  - **Can Run In Parallel**: NO (blocks all other tasks)
  - **Blocks**: Task 2-16

  **References**:
  - `tests/test_draw_vector.cpp:28` — 硬编码路径示例
  - `tests/test_helpers.h` — 需要添加路径辅助函数

  **Acceptance Criteria**:
  - [ ] `grep -r "D:/Projects" tests/` 返回空
  - [ ] 所有测试使用相对路径或 CMake 变量
  - [ ] `ctest --test-dir build --output-on-failure` 仍然通过

  **QA Scenarios**:
  ```
  Scenario: Verify no hardcoded paths
    Tool: Bash
    Steps:
      1. grep -r "D:/Projects" tests/
      2. Assert: no matches found
    Expected Result: Empty output
    Evidence: .sisyphus/evidence/task-01-no-hardcoded-paths.txt
  ```

  **Commit**: YES
  - Message: `fix(tests): replace hardcoded paths with relative paths`
  - Files: `tests/*.cpp`, `tests/test_helpers.h`

### Wave 3: Remaining Test Migration

- [ ] 2. Port SVG Rendering Tests (test_svg_render.cpp)

  **What to do**:
  - 从 LVGL `tests/src/test_cases/test_svg_file.c` 移植 SVG 渲染测试
  - 移除 `lv_draw_vector()` 调用，替换为直接 `vg_lite_draw()` 调用
  - 添加 SVG 测试资源到 `tests/assets/svg/`
  - 测试覆盖：
    - `test_svg_render_tiger` — tiger.svg 渲染
    - `test_svg_render_gradients` — 渐变测试
    - `test_svg_render_basic_shapes` — 基本形状

  **Must NOT do**:
  - NO 移植 SVG 解析器测试 (Task 13 已延后)
  - NO 创建新的 golden images

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 3, 4, 5)
  - **Blocked By**: Task 1

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\test_svg_file.c` — 源文件
  - `D:\Projects\lvgl-master\tests\src\test_assets\svg\` — SVG 测试资源

  **Acceptance Criteria**:
  - [ ] `tests/test_svg_render.cpp` 存在
  - [ ] 无 `lv_` 依赖
  - [ ] 至少 2 个 SVG 渲染测试通过

  **QA Scenarios**:
  ```
  Scenario: SVG render test compiles and runs
    Tool: Bash
    Steps:
      1. cmake --build build
      2. ctest --test-dir build -R SvgRender --output-on-failure
    Expected Result: Tests pass
    Evidence: .sisyphus/evidence/task-02-svg-render-tests.txt
  ```

  **Commit**: YES (groups with Wave 3)
  - Message: `test: port SVG rendering tests from LVGL`

- [ ] 3. Port Canvas Integration Tests (test_canvas_buffer.cpp)

  **What to do**:
  - 从 LVGL `tests/src/test_cases/widgets/test_canvas.c` 移植缓冲区相关测试
  - 测试覆盖：
    - `test_canvas_buffer_create` — 缓冲区分配
    - `test_canvas_buffer_stride` — stride 处理 (VGLite 特定)
    - `test_canvas_render_basic` — 基本画布渲染

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 2, 4, 5)
  - **Blocked By**: Task 1

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\widgets\test_canvas.c` — 源文件

  **Acceptance Criteria**:
  - [ ] `tests/test_canvas_buffer.cpp` 存在
  - [ ] 缓冲区操作正确

  **Commit**: YES (groups with Wave 3)

- [ ] 4. Port Color Format Tests (test_color_formats.cpp)

  **What to do**:
  - 从 LVGL `tests/src/test_cases/draw/test_render_to_*.c` 移植颜色格式测试
  - 测试覆盖格式：
    - ARGB8888, ARGB4444, ARGB1555
    - RGB565, RGB888
    - L8, AL88
  - 适配为使用 `vg_lite_buffer_t` 直接操作
  - 测试 `vg_lite_tvg.cpp` 中的格式转换逻辑

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 2, 3, 5)
  - **Blocked By**: Task 1

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\draw\test_render_to_*.c` — 12 个源文件

  **Acceptance Criteria**:
  - [ ] `tests/test_color_formats.cpp` 存在
  - [ ] 至少 4 种颜色格式测试通过

  **Commit**: YES (groups with Wave 3)

- [ ] 5. Verify and Document Golden Images

  **What to do**:
  - 验证现有 76 个 golden images 完整性
  - 创建 golden images 清单文件 `tests/ref_imgs/MANIFEST.md`
  - 文档每个 golden image 对应的测试用例
  - 确认不引用不存在的 `libs/` 和 `widgets/` 目录

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 2, 3, 4)
  - **Blocked By**: Task 1

  **References**:
  - `tests/ref_imgs/draw/` — 现有 golden images 目录

  **Acceptance Criteria**:
  - [ ] `tests/ref_imgs/MANIFEST.md` 存在
  - [ ] 76 个 golden images 已列出并文档化
  - [ ] 无对不存在目录的引用

  **Commit**: YES (groups with Wave 3)

### Wave 4: Test Execution

- [ ] 6. Verify LVGL VGLite Test Case Completeness (CRITICAL)

  **What to do**:
  - 扫描 `D:\Projects\lvgl-master\tests\src\test_cases\` 目录下所有涉及 VGLite 的测试文件
  - 对比当前项目 `tests/` 目录，识别缺失的测试用例
  - 必须覆盖的测试文件：
    - `draw/test_draw_vector.c` → `test_draw_vector.cpp` ✅
    - `draw/test_draw_vector_detail.c` → `test_draw_vector_detail.cpp` ✅
    - `draw/test_render_to_*.c` → `test_color_formats.cpp` (待确认)
    - `test_svg_file.c` → `test_svg_render.cpp` (待实现)
    - `widgets/test_canvas.c` → `test_canvas_buffer.cpp` (待实现)
  - 创建测试用例清单 `tests/TEST_COVERAGE.md`
  - 补充缺失的测试用例

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 2-5)
  - **Blocked By**: Task 2, 3, 4, 5

  **References**:
  - `D:\Projects\lvgl-master\tests\src\test_cases\` — LVGL 测试用例目录
  - `D:\Projects\lvgl-master\tests\ref_imgs_vg_lite\` — VGLite 专用 golden images

  **Acceptance Criteria**:
  - [ ] `tests/TEST_COVERAGE.md` 创建，列出所有 LVGL VGLite 测试用例
  - [ ] 所有 VGLite 相关测试用例已移植或说明原因
  - [ ] 无遗漏的核心测试用例

  **Commit**: YES
  - Message: `test: verify LVGL VGLite test case completeness`

- [ ] 7. Run and Fix All Ported Tests

  **What to do**:
  - 运行所有移植的测试
  - 修复编译错误
  - 修复运行时失败
  - **确保 Golden 比较有效执行**（与 LVGL 测试结果判断一致）
  - 文档任何跳过的测试及原因
  - **修复 `DrawBlendModesComparison` 禁用测试**（低优先级）

  **Golden 比较要求 (CRITICAL)**:
  - 测试用例执行结果判断必须与 LVGL 一致
  - 如果有 golden image 比较，必须实际执行像素级比较
  - 比较结果必须有意义（不能只是"存在文件"）
  - 使用与 LVGL 相同的误差容忍度

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 6)
  - **Blocked By**: Task 6

  **Acceptance Criteria**:
  - [ ] 所有移植测试编译通过
  - [ ] 核心矢量测试通过
  - [ ] **Golden 比较有效执行且有意义**
  - [ ] 已知问题已文档化
  - [ ] 测试通过率 ≥ 90%
  - [ ] `DrawBlendModesComparison` 修复（或文档说明原因）

  **QA Scenarios**:
  ```
  Scenario: All tests pass with valid golden comparison
    Tool: Bash
    Steps:
      1. cmake --build build --config Release
      2. ctest --test-dir build -C Release --output-on-failure
      3. Verify golden comparison logs show actual pixel comparison
    Expected Result: ≥90% tests pass, golden comparisons are meaningful
    Evidence: .sisyphus/evidence/task-07-test-results.txt
  ```

  **Commit**: YES
  - Message: `test: verify all ported tests passing`

- [ ] 8. Generate Coverage Report

  **What to do**:
  - 更新 `tests/COVERAGE_REPORT.md`
  - 添加新移植测试的覆盖率信息
  - 记录 API 函数覆盖情况
  - 识别未覆盖的关键函数

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 6)
  - **Blocked By**: Task 6

  **Acceptance Criteria**:
  - [ ] `tests/COVERAGE_REPORT.md` 更新
  - [ ] 新测试覆盖率已记录

  **Commit**: YES (groups with Wave 4)

### Wave 5a: ThorVG GL Backend Enablement

> ⚠️ **注意**: 当前 ThorVG 缺少 GL 渲染器核心源文件，需要从 LVGL 或完整 ThorVG 获取。

- [ ] 9. Fetch ThorVG GL engine source files

  **What to do**:
  - **优先从 LVGL 内置版本获取 GL 引擎源文件**
  - 来源顺序（禁止 GitHub 在线拉取）：
    1. `D:\Projects\lvgl-master\src\libs\thorvg\` (首选)
    2. `D:\Projects\opencode_plan_test\thorvg-main\thorvg-main\` (备用)
  - 需要获取的文件（如果存在于上述目录）：
    - `tvgGlRenderer.cpp`, `tvgGlRenderer.h`
    - `tvgGlShader.cpp`, `tvgGlShader.h`
    - `tvgGlGpuBuffer.cpp`, `tvgGlGpuBuffer.h`
    - `tvgGlGeometry.cpp`, `tvgGlGeometry.h`
    - `tvgGlProgram.cpp`, `tvgGlProgram.h`
    - `tvgGlCommon.h`
  - **如果 GL 引擎未实现，需要自行适配实现**
  - 复制到 `third_party/thorvg/` 目录

  **Must NOT do**:
  - NO 从 GitHub 在线拉取（可能失败）
  - NO 修改 ThorVG 源代码核心逻辑

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 10)
  - **Blocked By**: Task 8

  **References**:
  - `D:\Projects\lvgl-master\src\libs\thorvg\` — LVGL 内置 ThorVG（首选）
  - `D:\Projects\opencode_plan_test\thorvg-main\thorvg-main\` — 完整 ThorVG（备用）
  - `third_party/thorvg/tvgGlCanvas.cpp` — 现有空壳文件

  **Acceptance Criteria**:
  - [ ] GL 引擎源文件存在于 `third_party/thorvg/`
  - [ ] `tvgGlRenderer.h` 可被 `tvgGlCanvas.cpp` 包含
  - [ ] **如果源文件不存在，完成 GL 后端适配实现**

  **Commit**: YES (groups with Wave 5a)
  - Message: `feat(gl): add ThorVG GL engine source files`

- [ ] 10. Update ThorVG config for GL backend

  **What to do**:
  - 修改 `third_party/thorvg/config.h` 启用 GL 后端
  - 添加配置：
    ```c
    #define THORVG_GL_RASTER_SUPPORT 1
    #define THORVG_GL_GLES 1  // 用于 Android OpenGL ES
    ```
  - 保留 SW 后端支持（两者并存）

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 9)
  - **Blocked By**: Task 8

  **References**:
  - `third_party/thorvg/config.h` — ThorVG 配置文件

  **Acceptance Criteria**:
  - [ ] `THORVG_GL_RASTER_SUPPORT` 定义为 1
  - [ ] `THORVG_GL_GLES` 定义为 1

  **Commit**: YES (groups with Wave 5a)

- [ ] 11. Update CMakeLists.txt for GL sources

  **What to do**:
  - 更新 `third_party/CMakeLists.txt` 包含 GL 渲染器源文件
  - 添加条件编译：当 `VG_LITE_USE_GL_BACKEND=ON` 时包含 GL 源文件
  - 添加 GL 源文件：
    - `tvgGlRenderer.cpp`
    - `tvgGlShader.cpp`
    - `tvgGlGpuBuffer.cpp`
    - `tvgGlCanvas.cpp`
    - `tvgGlGeometry.cpp`
    - `tvgGlProgram.cpp`
  - 添加 OpenGL 链接库

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 9, 10)
  - **Blocked By**: Task 8

  **References**:
  - `third_party/thorvg/` — GL 源文件目录
  - `third_party/CMakeLists.txt` — 需要更新的构建文件

  **Acceptance Criteria**:
  - [ ] GL 源文件在 `VG_LITE_USE_GL_BACKEND=ON` 时被编译
  - [ ] Windows 构建链接 OpenGL32.lib

  **Commit**: YES (groups with Wave 5a)

- [ ] 12. Add GL backend macro support

  **What to do**:
  - 更新 `src/vg_lite_tvg.cpp` 支持 GL 后端选择
  - 确保宏切换正确：
    ```cpp
    #if VG_LITE_RENDER_BACKEND == VG_LITE_RENDER_GL
      #define TVG_CANVAS_ENGINE CanvasEngine::Gl
      // GL 初始化代码
      #include "tvgGlCanvas.h"
    #else
      #define TVG_CANVAS_ENGINE CanvasEngine::Sw
    #endif
    ```
  - 添加 GL 后端初始化逻辑
  - 测试 GL Canvas 创建

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 9, 10, 11)
  - **Blocked By**: Task 8

  **References**:
  - `src/vg_lite_tvg.cpp` — 需要更新的核心文件
  - `third_party/thorvg/tvgGlCanvas.cpp` — GL Canvas API

  **Acceptance Criteria**:
  - [ ] `VG_LITE_RENDER_GL` 宏定义存在
  - [ ] GL 后端条件编译正确

  **Commit**: YES (groups with Wave 5a)

- [ ] 13. Test GL backend on Windows

  **What to do**:
  - 使用 `-DVG_LITE_USE_GL_BACKEND=ON` 构建
  - 创建简单的 GL 后端测试
  - 验证 OpenGL 3.3 上下文创建
  - 验证基本渲染功能

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 9, 10, 11, 12)
  - **Blocked By**: Task 9, 10, 11, 12

  **References**:
  - `examples/hello_path.c` — 可用于测试的示例

  **Acceptance Criteria**:
  - [ ] `cmake -B build -DVG_LITE_USE_GL_BACKEND=ON` 成功
  - [ ] GL 后端编译无错误
  - [ ] 简单 GL 渲染测试通过

  **QA Scenarios**:
  ```
  Scenario: GL backend builds successfully
    Tool: Bash
    Steps:
      1. cmake -B build-gl -DVG_LITE_USE_GL_BACKEND=ON
      2. cmake --build build-gl --config Release
    Expected Result: Build succeeds
    Evidence: .sisyphus/evidence/task-13-gl-build.txt
  ```

  **Commit**: YES
  - Message: `test(gl): verify GL backend compiles and runs on Windows`

### Wave 5b: Android Adaptation

- [ ] 14. Create Android project structure

  **What to do**:
  - 创建 `android/` 目录结构：
    ```
    android/
    ├── app/
    │   ├── src/main/
    │   │   ├── AndroidManifest.xml
    │   │   ├── cpp/
    │   │   │   ├── main.cpp
    │   │   │   └── vglite_renderer.cpp
    │   │   └── res/
    │   ├── build.gradle
    │   └── CMakeLists.txt
    ├── jni/
    │   ├── vglite_jni.cpp
    │   └── CMakeLists.txt
    ├── build.gradle
    ├── settings.gradle
    └── gradle.properties
    ```
  - 创建 `AndroidManifest.xml`
  - 配置最小 SDK 版本 (API 21)

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 15)
  - **Blocked By**: Task 13

  **References**:
  - https://github.com/thorvg/thorvg.android — 官方 Android 构建参考

  **Acceptance Criteria**:
  - [ ] `android/` 目录结构完整
  - [ ] `AndroidManifest.xml` 配置正确

  **Commit**: YES (groups with Wave 5b)
  - Message: `feat(android): create project structure`

- [ ] 15. Create Android NDK CMakeLists.txt

  **What to do**:
  - 创建 `android/app/CMakeLists.txt` 用于 NDK 构建
  - 配置 Android 工具链支持
  - 配置 OpenGL ES 3.1 链接
  - 配置 ThorVG GL 后端编译
  - 添加必要的 NDK 库：`GLESv3`, `EGL`, `android`, `log`

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 14)
  - **Blocked By**: Task 13

  **References**:
  - `CMakeLists.txt` — 根项目构建文件
  - Android NDK CMake 文档

  **Acceptance Criteria**:
  - [ ] Android CMakeLists.txt 配置正确
  - [ ] 链接库正确配置

  **Commit**: YES (groups with Wave 5b)

- [ ] 16. Create JNI wrapper layer

  **What to do**:
  - 创建 `android/jni/vglite_jni.cpp` JNI 封装
  - 封装核心 VGLite API：
    - `Java_com_example_vglite_VGLite_nativeInit()`
    - `Java_com_example_vglite_VGLite_nativeDraw()`
    - `Java_com_example_vglite_VGLite_nativeClose()`
  - 处理 JNI 类型转换
  - 添加错误处理和日志

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 14, 15)
  - **Blocked By**: Task 14, 15

  **References**:
  - `include/vg_lite.h` — VGLite API 头文件
  - JNI 文档

  **Acceptance Criteria**:
  - [ ] `vglite_jni.cpp` 存在
  - [ ] JNI 函数导出正确
  - [ ] 编译无错误

  **Commit**: YES (groups with Wave 5b)

- [ ] 17. Create Android demo app (native)

  **What to do**:
  - 使用 `android_native_app_glue` 模式创建原生应用
  - 创建 `android/app/src/main/cpp/main.cpp`
  - 实现 OpenGL ES 3.1 上下文初始化
  - 实现简单渲染循环：绘制三角形或矩形
  - 实现 EGL 窗口管理
  - 确保应用正常退出

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
  - **Skills**: []
  - Reason: 涉及 OpenGL ES 渲染和 UI 集成

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 16)
  - **Blocked By**: Task 16

  **References**:
  - Android NDK native_app_glue 示例
  - `android-ndk/examples/native-activity/`

  **Acceptance Criteria**:
  - [ ] 原生应用源文件存在
  - [ ] OpenGL ES 上下文创建成功
  - [ ] 简单图形渲染正常
  - [ ] 应用正常退出

  **QA Scenarios**:
  ```
  Scenario: Android app builds
    Tool: Bash
    Steps:
      1. cd android && ./gradlew assembleDebug
    Expected Result: APK generated
    Evidence: .sisyphus/evidence/task-17-android-build.txt

  Scenario: Android app runs on emulator
    Tool: Bash
    Steps:
      1. adb install app/build/outputs/apk/debug/app-debug.apk
      2. adb shell am start -n com.example.vglite/.MainActivity
      3. adb logcat -d | grep "vglite"
    Expected Result: App starts, renders, exits cleanly
    Evidence: .sisyphus/evidence/task-17-android-run.txt
  ```

  **Commit**: YES (groups with Wave 5b)
  - Message: `feat(android): create native demo app`

- [ ] 18. Build and test on Android

  **What to do**:
  - 使用 Android NDK 构建项目
  - 在 Android 模拟器上测试
  - 验证 OpenGL ES 3.1 渲染
  - 修复任何 Android 特定问题

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 17)
  - **Blocked By**: Task 17

  **References**:
  - Android NDK 构建文档

  **Acceptance Criteria**:
  - [ ] `libvglite.so` 成功生成
  - [ ] APK 成功生成
  - [ ] 应用在模拟器上启动并渲染

  **Commit**: YES
  - Message: `test(android): verify build and run on Android`

---

## Final Verification Wave (MANDATORY)

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Verify all "Must Have" implemented, all "Must NOT Have" absent.

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Run build, check for warnings/errors.

- [ ] F3. **Real Manual QA** — `unspecified-high`
  Run Android demo on emulator/device.

- [ ] F4. **Test Coverage Report** — `unspecified-high`
  Report test coverage from all ported tests.

---

## Commit Strategy

- **Wave 0**: `fix(tests): replace hardcoded paths with relative paths`
- **Wave 3**: `test: port SVG/Canvas/ColorFormat tests from LVGL`
- **Wave 4**: `test: verify all tests passing, generate coverage`
- **Wave 5a**: `feat(gl): enable ThorVG OpenGL backend`
- **Wave 5b**: `feat(android): add Android NDK build and demo app`

---

## Success Criteria

### Verification Commands
```bash
# Windows build
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# Android build
cmake -B build-android \
  -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-21
cmake --build build-android

# Verify GL backend
grep "THORVG_GL_RASTER_SUPPORT" third_party/thorvg/config.h
# Expected: #define THORVG_GL_RASTER_SUPPORT 1
```

### Final Checklist
- [ ] All hardcoded paths fixed
- [ ] All ported tests pass (17/17 or 16/17 with 1 disabled documented)
- [ ] ThorVG GL backend enabled and compiling
- [ ] Android NDK build succeeds
- [ ] Android demo runs on emulator/device
- [ ] No new LVGL dependencies
- [ ] Coverage report generated
