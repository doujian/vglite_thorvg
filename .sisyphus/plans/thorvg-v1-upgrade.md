# ThorVG v1.0.0 升级计划

## TL;DR

> **Quick Summary**: 将 ThorVG 从 v0.15.3 升级到 v1.0.0，适配 SW 后端，新增 dump 功能。
> 
> **Deliverables**:
> - 确保当前测试全部通过 (43/43)
> - 替换 ThorVG v0.15.3 → v1.0.0 (SW 后端)
> - 更新适配层 `vg_lite_tvg.cpp` 中的 API 转换函数
> - 新增 `vg_lite_dump_buffer()` API 输出 PNG
> - 调整 golden images 容忍度
> 
> **Estimated Effort**: Medium
> **Parallel Execution**: YES - 4 waves
> **Critical Path**: Task 1 → Task 3 → Task 7 → Task 10 → Task 12 → F1-F4

---

## Context

### Original Request
1. 先保证当前版本用例 pass
2. 分析版本差异，适配到 ThorVG v1.0.0，先适配 SW 后端
3. 新增支持 ThorVG GL 后端
4. 单元测试执行完后增加 dump 功能

### 用户确认的选择
- **升级策略**: 完全替换到 v1.0.0
- **Dump 功能**: 新 API `vg_lite_dump_buffer()`，仅输出 PNG
- **API 兼容性**: 完全兼容现有 vg_lite_* API
- **Golden Images**: 先调整容忍度，后续再更新
- **GL 后端**: 暂不启用（本次只适配 SW 后端）

### Interview Summary

**Key Discussions**:
- 升级策略：完全替换到 v1.0.0（删除旧版本，直接使用 ThorVG v1.0.0）
- Dump 功能：输出渲染结果为 PNG（使用 stb_image_write）
- API 兼容性：完全兼容现有 vg_lite_* API
- Golden Images：两者结合（先调整容忍度让测试通过，后续再更新）
- GL 后端：暂不启用 GL（延后实现）

**Research Findings**:
- 当前版本: ThorVG v0.15.3-vglite (LVGL 修改版)
- 新版本: ThorVG v1.0.0 (位于 D:\Projects\thorvg-main)
- v1.0.0 API 变化: `stroke()` → `strokeWidth()`, `CompositeMethod` → `MaskMethod`, `CanvasEngine` 枚举移除
- 当前测试: 44 个用例，43 通过，1 禁用 (DrawBlendModesComparison)
- 关键适配层: `src/vg_lite_tvg.cpp` (3127 行，60+ vg_lite API 映射)

### Metis Review

**Identified Gaps** (addressed):
- **API 变化**: v1.0 有 ~8 个关键 API 变化，需更新 `vg_lite_tvg.cpp` 中的转换函数
- **Golden Images**: 渲染算法差异可能导致像素级差异，需调整容忍度
- **FillRule 命名**: `Winding` → `NonZero`，需更新映射
- **BlendMethod 新增**: v1.0 新增 Add/Subtract/Intersect/Difference/Lighten/Darken

---

## Work Objectives

### Core Objective
升级 ThorVG 到 v1.0.0，保持 vg_lite API 完全兼容，新增 dump 功能。

### Concrete Deliverables
- 所有测试通过 (43/43，含 1 个禁用测试)
- `third_party/thorvg/` 替换为 v1.0.0 源码
- `src/vg_lite_tvg.cpp` 更新适配 v1.0 API
- `include/vg_lite.h` 新增 `vg_lite_dump_buffer()` 声明
- `src/vg_lite_dump.cpp` 实现 PNG 输出
- 测试容忍度调整配置

### Definition of Done
- [ ] `ctest --test-dir build --output-on-failure` 全部通过 (43/43)
- [ ] `vg_lite_dump_buffer()` API 可用并测试通过
- [ ] 无 vg_lite API 破坏性变更

### Must Have
- 替换 ThorVG v0.15.3 → v1.0.0
- 更新所有 API 转换函数适配 v1.0
- 新增 dump 功能
- 测试全部通过
- vg_lite_* API 完全兼容

### Must NOT Have (Guardrails)
- NO 修改测试文件的测试逻辑（测试验证 API 兼容性）
- NO 启用 GL 后端（延后）
- NO 修改 vg_lite 公共 API 签名
- NO 跳过任何测试
- NO 增加 BMP 格式支持（仅 PNG）

---

## Verification Strategy (MANDATORY)

> **ZERO HUMAN INTERVENTION** — ALL verification is agent-executed.

### Test Decision
- **Infrastructure exists**: YES (Google Test)
- **Automated tests**: YES
- **Framework**: Google Test (C++)

### QA Policy
Every task MUST include agent-executed QA scenarios.
Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Foundation — baseline verification):
├── Task 1: Verify current test baseline [quick]
└── Task 2: Document API changes in v1.0 [quick]

Wave 2 (ThorVG Upgrade — SW backend migration):
├── Task 3: Backup and replace ThorVG source [quick]
├── Task 4: Update CMakeLists.txt for v1.0 structure [quick]
├── Task 5: Update blend_method_conv() [quick]
├── Task 6: Update fill_rule_conv() [quick]
├── Task 7: Update stroke APIs (strokeWidth, etc.) [quick]
├── Task 8: Update Initializer::init() (remove CanvasEngine) [quick]
└── Task 9: Update CompositeMethod → MaskMethod [unspecified-high]

Wave 3 (Validation — test and fix):
├── Task 10: Run tests and fix compilation errors [deep]
├── Task 11: Adjust golden image tolerance [quick]
└── Task 12: Verify all tests pass [deep]

Wave 4 (Dump Feature — PNG output):
├── Task 13: Add vg_lite_dump_buffer() declaration [quick]
├── Task 14: Implement dump functionality [quick]
└── Task 15: Test dump feature [quick]

Wave FINAL (After ALL tasks — verification):
├── Task F1: Plan compliance audit [oracle]
├── Task F2: Code quality review [unspecified-high]
├── Task F3: API compatibility test [unspecified-high]
└── Task F4: Golden images validation [deep]
-> Present results -> Get explicit user okay

Critical Path: Task 1 → Task 3 → Task 7 → Task 10 → Task 12 → Task 15 → F1-F4
Parallel Speedup: ~50% faster than sequential
Max Concurrent: 7 (Wave 2)
```

### Dependency Matrix

| Task | Depends On | Blocks |
|------|------------|--------|
| 1 | — | 3-15 |
| 2 | — | 5-9 |
| 3 | 1 | 10-15 |
| 4 | 3 | 10 |
| 5-9 | 2, 3 | 10 |
| 10 | 4-9 | 11-12 |
| 11 | 10 | 12 |
| 12 | 11 | 13-15 |
| 13 | 12 | 14 |
| 14 | 13 | 15 |
| 15 | 14 | F1-F4 |
| F1-F4 | 15 | — |

---

## TODOs

### Wave 1: Foundation — Baseline Verification

- [x] 1. Verify current test baseline

  **What to do**:
  - 运行 `ctest --test-dir build --output-on-failure` 确认当前测试状态
  - 记录 43 个通过测试和 1 个禁用测试的详细信息
  - 文档禁用测试 `DrawBlendModesComparison` 的原因

  **Must NOT do**:
  - NO 修改任何代码
  - NO 尝试修复禁用测试

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 2)
  - **Blocks**: Task 3-15

  **References**:
  - `tests/test_draw_vector_detail.cpp:95` — 禁用测试位置
  - `CMakeLists.txt` — 构建配置

  **Acceptance Criteria**:
  - [ ] 测试状态文档化：43 通过，1 禁用
  - [ ] 禁用测试原因已记录

  **QA Scenarios**:
  ```
  Scenario: Verify test baseline
    Tool: Bash
    Steps:
      1. cmake --build build --config Release
      2. ctest --test-dir build -C Release --output-on-failure
    Expected Result: 43/44 tests pass, 1 disabled
    Evidence: .sisyphus/evidence/task-01-test-baseline.txt
  ```

  **Commit**: NO (verification only)

- [x] 2. Document API changes in ThorVG v1.0

  **What to do**:
  - 创建 `docs/THORVG_V1_MIGRATION.md` 文档
  - 记录所有 API 变化：
    - `stroke()` → `strokeWidth()`
    - `CompositeMethod` → `MaskMethod` + `clip()`
    - `CanvasEngine` 枚举移除
    - `FillRule::Winding` → `FillRule::NonZero`
    - `BlendMethod` 新增：Add, Subtract, Intersect, Difference, Lighten, Darken
  - 标注 `vg_lite_tvg.cpp` 中需要更新的函数位置

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 1)
  - **Blocks**: Task 5-9

  **References**:
  - `D:\Projects\thorvg-main\meson.build` — v1.0 版本信息
  - `src/vg_lite_tvg.cpp` — 需要更新的适配层
  - ThorVG v1.0 CHANGELOG

  **Acceptance Criteria**:
  - [ ] `docs/THORVG_V1_MIGRATION.md` 存在
  - [ ] 所有关键 API 变化已记录
  - [ ] 需要更新的函数位置已标注

  **Commit**: YES
  - Message: `docs: document ThorVG v1.0 API migration changes`
  - Files: `docs/THORVG_V1_MIGRATION.md`

### Wave 2: ThorVG Upgrade — SW Backend Migration

- [x] 3. Backup and replace ThorVG source

  **What to do**:
  - 创建备份目录 `third_party/thorvg_v0.15_backup/`
  - 复制当前 `third_party/thorvg/` 内容到备份目录
  - 从 `D:\Projects\thorvg-main` 复制 v1.0 源码到 `third_party/thorvg/`
  - 保留 `config.h` 配置文件（稍后更新）

  **Must NOT do**:
  - NO 删除备份
  - NO 修改 config.h（Task 4 处理）

  **Parallelization**:
  - **Can Run In Parallel**: NO (blocks Task 4-9)
  - **Blocked By**: Task 1

  **References**:
  - `third_party/thorvg/` — 当前 v0.15 源码
  - `D:\Projects\thorvg-main\` — v1.0 源码位置
  - `third_party/thorvg/config.h` — 需保留的配置

  **Acceptance Criteria**:
  - [ ] `third_party/thorvg_v0.15_backup/` 备份存在
  - [ ] `third_party/thorvg/` 包含 v1.0 源码
  - [ ] `config.h` 保留原有配置

  **Commit**: YES
  - Message: `refactor: replace ThorVG v0.15 with v1.0.0`
  - Files: `third_party/thorvg/`

- [x] 4. Update CMakeLists.txt for v1.0 structure

  **What to do**:
  - 更新 `third_party/CMakeLists.txt` 适配 v1.0 目录结构
  - v1.0 结构：`common/`, `renderer/sw_engine/`, `loaders/`
  - 更新源文件列表
  - 确保 SW 后端编译选项正确

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 5-9)
  - **Blocked By**: Task 3

  **References**:
  - `third_party/CMakeLists.txt` — 需要更新的构建文件
  - `D:\Projects\thorvg-main\meson.build` — v1.0 构建参考

  **Acceptance Criteria**:
  - [ ] CMakeLists.txt 更新完成
  - [ ] `cmake -B build` 成功
  - [ ] SW 后端源文件正确包含

  **Commit**: YES (groups with Wave 2)
  - Message: `build: update CMakeLists.txt for ThorVG v1.0 structure`

- [x] 5. Update blend_method_conv()

  **What to do**:
  - 更新 `src/vg_lite_tvg.cpp` 中的 `blend_method_conv()` 函数
  - 映射 v1.0 新增的 BlendMethod：
    - Add, Subtract, Intersect, Difference, Lighten, Darken
  - 确保所有 vg_lite_blend_mode_t 正确映射

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 4, 6-9)
  - **Blocked By**: Task 2, 3

  **References**:
  - `src/vg_lite_tvg.cpp` — 需要更新的文件
  - `include/vg_lite.h:vg_lite_blend_mode_t` — vg_lite 混合模式定义
  - `D:\Projects\thorvg-main\src\common\tvgCommon.h` — ThorVG BlendMethod 定义

  **Acceptance Criteria**:
  - [ ] `blend_method_conv()` 更新完成
  - [ ] 所有混合模式正确映射

  **Commit**: YES (groups with Wave 2)

- [x] 6. Update fill_rule_conv()

  **What to do**:
  - 更新 `src/vg_lite_tvg.cpp` 中的 `fill_rule_conv()` 函数
  - 映射 FillRule 变化：
    - `FillRule::Winding` → `FillRule::NonZero`
    - `FillRule::EvenOdd` 保持不变

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 4-5, 7-9)
  - **Blocked By**: Task 2, 3

  **References**:
  - `src/vg_lite_tvg.cpp` — 需要更新的文件
  - `include/vg_lite.h:vg_lite_fill_t` — vg_lite 填充规则定义

  **Acceptance Criteria**:
  - [ ] `fill_rule_conv()` 更新完成
  - [ ] Winding/EvenOdd 正确映射到 NonZero/EvenOdd

  **Commit**: YES (groups with Wave 2)

- [x] 7. Update stroke APIs

  **What to do**:
  - 更新 `src/vg_lite_tvg.cpp` 中的 stroke 相关调用：
    - `stroke(width)` → `strokeWidth(width)`
    - `stroke(color)` → `strokeColor(color)`
    - `stroke(params)` → `strokeWidth()/strokeColor()` 分离调用
  - 检查所有使用 `stroke()` 的位置并更新

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 4-6, 8-9)
  - **Blocked By**: Task 2, 3

  **References**:
  - `src/vg_lite_tvg.cpp` — 需要更新的文件
  - ThorVG v1.0 API 文档

  **Acceptance Criteria**:
  - [ ] 所有 `stroke()` 调用更新为 v1.0 API
  - [ ] 编译无警告

  **Commit**: YES (groups with Wave 2)

- [x] 8. Update Initializer::init()

  **What to do**:
  - 更新 `vg_lite_init()` 中的 ThorVG 初始化代码
  - v1.0 移除了 `CanvasEngine` 枚举
  - 更新初始化调用：
    ```cpp
    // v0.15: Initializer::init(CanvasEngine::Sw, threads)
    // v1.0:  Initializer::init(threads)  // SW 是默认后端
    ```
  - 更新 `vg_lite_close()` 中的清理代码

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 4-7, 9)
  - **Blocked By**: Task 2, 3

  **References**:
  - `src/vg_lite_tvg.cpp:vg_lite_init()` — 初始化函数
  - `src/vg_lite_tvg.cpp:vg_lite_close()` — 清理函数

  **Acceptance Criteria**:
  - [x] `vg_lite_init()` 更新完成
  - [x] `vg_lite_close()` 更新完成
  - [x] 编译无错误

  **Commit**: YES (groups with Wave 2)

- [x] 9. Update CompositeMethod → MaskMethod

  **What to do**:
  - 更新 `src/vg_lite_tvg.cpp` 中的组合方法相关代码
  - v1.0 拆分为 `MaskMethod` + `clip()`：
    - 复合操作改用 `mask(MaskMethod)`
    - 裁剪操作改用 `clip()`
  - 更新 `composite_method_conv()` 转换函数

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 4-8)
  - **Blocked By**: Task 2, 3

  **References**:
  - `src/vg_lite_tvg.cpp` — 需要更新的文件
  - `include/vg_lite.h` — vg_lite 组合模式定义
  - ThorVG v1.0 API 文档

  **Acceptance Criteria**:
  - [ ] 组合方法相关代码更新完成
  - [ ] 所有组合操作正确映射

  **Commit**: YES (groups with Wave 2)
  - Message: `refactor: update ThorVG API calls for v1.0 compatibility`

### Wave 3: Validation — Test and Fix

- [x] 10. Run tests and fix compilation errors

  **What to do**:
  - 运行 `cmake --build build` 编译项目
  - 修复所有编译错误
  - 修复所有编译警告
  - 确保项目可以成功编译

  **Must NOT do**:
  - NO 跳过任何编译错误
  - NO 使用 `#pragma warning disable` 隐藏警告

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Wave 2)
  - **Blocked By**: Task 4-9

  **References**:
  - `src/vg_lite_tvg.cpp` — 主要适配层
  - `third_party/thorvg/` — ThorVG 源码

  **Acceptance Criteria**:
  - [ ] `cmake --build build --config Release` 成功
  - [ ] 无编译错误
  - [ ] 无编译警告（或警告已合理处理）

  **QA Scenarios**:
  ```
  Scenario: Build succeeds
    Tool: Bash
    Steps:
      1. cmake -B build -G "Visual Studio 17 2022"
      2. cmake --build build --config Release
    Expected Result: Build succeeds with 0 errors
    Evidence: .sisyphus/evidence/task-10-build.txt
  ```

  **Commit**: YES
  - Message: `fix: resolve compilation errors after ThorVG upgrade`

- [x] 11. Adjust golden image tolerance

  **What to do**:
  - 运行测试，分析 golden image 比较失败原因
  - 在 `tests/test_helpers.h` 或相关文件中调整容忍度
  - 可能需要修改 `compare_images()` 函数的误差阈值
  - 建议容忍度：5-10% 像素差异（根据实际测试调整）

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 10)
  - **Blocked By**: Task 10

  **References**:
  - `tests/test_helpers.h` — 测试辅助函数
  - `tests/test_golden.cpp` — golden 测试
  - `tests/stb/` — 图像比较库

  **Acceptance Criteria**:
  - [ ] 容忍度参数可配置
  - [ ] 测试输出显示实际差异百分比
  - [ ] 容忍度调整后测试通过

  **Commit**: YES
  - Message: `test: adjust golden image tolerance for ThorVG v1.0`

- [x] 12. Verify all tests pass

  **What to do**:
  - 运行完整测试套件
  - 确认 43 个测试通过
  - 分析并修复任何失败的测试
  - 文档 1 个禁用测试的状态

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 11)
  - **Blocked By**: Task 11

  **References**:
  - `tests/test_*.cpp` — 所有测试文件
  - `tests/ref_imgs/` — golden images

  **Acceptance Criteria**:
  - [ ] `ctest --test-dir build --output-on-failure` 显示 43/44 通过
  - [ ] 1 个禁用测试已文档化
  - [ ] 无意外失败的测试

  **QA Scenarios**:
  ```
  Scenario: All tests pass
    Tool: Bash
    Steps:
      1. ctest --test-dir build -C Release --output-on-failure
    Expected Result: 43/44 tests pass, 1 disabled
    Evidence: .sisyphus/evidence/task-12-test-results.txt
  ```

  **Commit**: YES
  - Message: `test: verify all tests pass with ThorVG v1.0`

### Wave 4: Dump Feature — PNG Output

- [x] 13. Add vg_lite_dump_buffer() declaration

  **What to do**:
  - 在 `include/vg_lite.h` 中添加新 API 声明：
    ```c
    /**
     * @brief Dump buffer content to PNG file
     * @param buffer Pointer to vg_lite_buffer_t
     * @param filename Output PNG file path
     * @return VG_LITE_SUCCESS on success, error code otherwise
     */
    vg_lite_error_t vg_lite_dump_buffer(
        vg_lite_buffer_t* buffer,
        const char* filename
    );
    ```
  - 添加必要的头文件包含（如 `stddef.h`）

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 12)
  - **Blocked By**: Task 12

  **References**:
  - `include/vg_lite.h` — API 头文件
  - `include/vg_lite.h:vg_lite_error_t` — 错误码定义
  - `include/vg_lite.h:vg_lite_buffer_t` — 缓冲区结构

  **Acceptance Criteria**:
  - [ ] `vg_lite_dump_buffer()` 声明存在
  - [ ] 函数签名正确
  - [ ] 文档注释完整

  **Commit**: YES (groups with Wave 4)

- [x] 14. Implement dump functionality

  **What to do**:
  - 创建 `src/vg_lite_dump.cpp` 实现文件
  - 使用 `stb_image_write.h` 实现 PNG 输出
  - 实现 `vg_lite_dump_buffer()` 函数：
    - 支持 ARGB8888 格式
    - 转换其他格式到 ARGB8888
    - 调用 `stbi_write_png()` 输出
  - 更新 `CMakeLists.txt` 包含新源文件

  **Must NOT do**:
  - NO 支持 BMP 格式（仅 PNG）
  - NO 添加外部依赖（使用现有 stb 库）

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 13)
  - **Blocked By**: Task 13

  **References**:
  - `tests/stb/stb_image_write.h` — PNG 输出库
  - `src/vg_lite_tvg.cpp` — 参考格式转换逻辑
  - `include/vg_lite.h:vg_lite_buffer_t` — 缓冲区结构

  **Acceptance Criteria**:
  - [ ] `src/vg_lite_dump.cpp` 存在
  - [ ] 支持 ARGB8888 格式输出
  - [ ] 编译无错误
  - [ ] PNG 文件正确生成

  **Commit**: YES (groups with Wave 4)
  - Message: `feat: add vg_lite_dump_buffer() API for PNG output`

- [ ] 15. Test dump feature

  **What to do**:
  - 创建简单测试验证 dump 功能
  - 在现有测试的 teardown 中添加可选 dump 调用
  - 验证生成的 PNG 文件：
    - 文件存在
    - 文件大小合理
    - 可被图像查看器打开

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 14)
  - **Blocked By**: Task 14

  **References**:
  - `tests/test_main.cpp` — 可添加 dump 测试
  - `tests/stb/stb_image.h` — 可用于验证 PNG

  **Acceptance Criteria**:
  - [ ] dump 测试通过
  - [ ] PNG 文件正确生成
  - [ ] PNG 文件可打开

  **QA Scenarios**:
  ```
  Scenario: Dump feature works
    Tool: Bash
    Steps:
      1. cmake --build build --config Release
      2. ./build/tests/test_dump.exe
      3. ls -la output.png
    Expected Result: PNG file exists and size > 0
    Evidence: .sisyphus/evidence/task-15-dump-test.png
  ```

  **Commit**: YES
  - Message: `test: add tests for vg_lite_dump_buffer()`

---

## Final Verification Wave (MANDATORY)

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Verify all "Must Have" implemented, all "Must NOT Have" absent.

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Run build, check for warnings/errors.

- [ ] F3. **API Compatibility Test** — `unspecified-high`
  Verify all vg_lite_* APIs work correctly.

- [ ] F4. **Golden Images Validation** — `deep`
  Validate all tests pass with adjusted tolerance.

---

## Commit Strategy

- **Wave 1**: `docs: document ThorVG v1.0 API changes`
- **Wave 2**: `refactor: upgrade ThorVG to v1.0.0`
- **Wave 3**: `test: adjust golden image tolerance for v1.0`
- **Wave 4**: `feat: add vg_lite_dump_buffer() API`

---

## Success Criteria

### Verification Commands
```bash
# Build and test
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# Verify dump API
cmake --build build --config Release
./build/tests/test_dump.exe
# Expected: PNG file generated
```

### Final Checklist
- [ ] All tests pass (43/43)
- [ ] ThorVG v1.0.0 integrated
- [ ] vg_lite_dump_buffer() works
- [ ] No API breaking changes
- [ ] Golden images valid with tolerance