# Add GL Context to Test Framework

## TL;DR

> **Quick Summary**: Add OpenGL context creation to the test framework so that GL backend tests can run without being skipped. This will prove that the skipped tests work correctly when a valid GL context exists.
> 
> **Deliverables**:
> - GLContextManager class in test_helpers.h
> - GL context initialization in test_main.cpp
> - Updated test files that run (not skip) on GL backend
> - All tests passing on both SW and GL backends

---

## Context

### Problem Statement

Currently, 12 tests are skipped on GL backend because there's no valid OpenGL context when tests run. The tests use `shouldSkipRenderingTests()` to skip, but we need to prove these tests actually work when a GL context exists.

### Current State

**SW Backend**: 100% passing (64/64 tests)
**GL Backend**: 100% passing but with 12 skipped tests

**Skipped Tests on GL Backend**:
- `VectorDrawTest.DrawLines`
- `VectorDrawTest.DrawShapes`
- `VectorDetailTest.DrawCopyPath`
- `VectorDetailTest.DrawRectPath`
- `VectorDetailTest.DrawAppendPath`
- `VectorDetailTest.DrawArcPath`
- `GLBackendTest.UploadBuffer`
- `GLBackendTest.UploadYUVBuffer`
- `GLBackendTest.Integration_CompleteWorkflow`
- `GoldenImageTest.VectorDrawShapes`
- `GoldenImageTest.BlendModesComparison`
- `GoldenImageTest.FillRulesComparison`
- `GoldenImageTest.MatrixTransformComparison`

### Solution Approach

Create a hidden window with OpenGL context on Windows using WGL API. Initialize this context before tests run, so GL backend tests can execute properly.

---

## Work Objectives

### Core Objective
Prove that GL backend tests work correctly by providing a valid OpenGL context.

### Concrete Deliverables
- `tests/test_helpers.h`: GLContextManager class with WGL initialization
- `tests/test_main.cpp`: GL context initialization before tests
- `tests/test_draw_vector.cpp`: Remove skip logic
- `tests/test_draw_vector_detail.cpp`: Remove skip logic
- `tests/test_golden.cpp`: Remove skip logic
- `tests/test_gl_backend.cpp`: Remove skip logic

### Definition of Done
- [ ] GLContextManager implemented and working
- [ ] All tests run (not skipped) on GL backend
- [ ] All tests pass on both SW and GL backends
- [ ] No SEH exceptions or crashes

---

## Verification Strategy

### Test Strategy
- **Infrastructure exists**: Yes (Google Test)
- **Automated tests**: Tests-after (run existing test suite)
- **Framework**: ctest

### QA Scenarios
- Run `ctest --output-on-failure` on GL backend
- Verify no tests are skipped
- Verify all tests pass

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Foundation):
├── Task 1: Add GLContextManager to test_helpers.h
└── Task 2: Update test_main.cpp to initialize GL context

Wave 2 (Remove Skip Logic):
├── Task 3: Update test_draw_vector.cpp
├── Task 4: Update test_draw_vector_detail.cpp
├── Task 5: Update test_golden.cpp
└── Task 6: Update test_gl_backend.cpp

Wave 3 (Verification):
├── Task 7: Build and test SW backend
└── Task 8: Build and test GL backend
```

---

## TODOs

- [ ] 1. Add GLContextManager to test_helpers.h

  **What to do**:
  - Add GLContextManager class with Windows WGL support
  - Implement `initialize()` to create hidden window + GL context
  - Implement `cleanup()` to release resources
  - Implement `hasContext()` to check if context exists
  - Make it a singleton with static `instance()` method

  **Code to add**:
  ```cpp
  class GLContextManager {
  public:
      static GLContextManager& instance();
      bool initialize();  // Creates hidden window + GL context
      void cleanup();     // Releases resources
      bool hasContext() const;
  private:
      GLContextManager() = default;
      ~GLContextManager() { cleanup(); }
      // Windows handles: HINSTANCE, HWND, HDC, HGLRC
  };
  ```

  **References**:
  - `tests/test_helpers.h:34-57` - Current hasGLContext() implementation
  - Windows WGL API documentation

  **Acceptance Criteria**:
  - [ ] GLContextManager class compiles
  - [ ] initialize() returns true on GL backend
  - [ ] hasContext() returns true after initialize()

- [ ] 2. Update test_main.cpp to initialize GL context

  **What to do**:
  - Add `#include "test_helpers.h"` if not present
  - Call `GLContextManager::instance().initialize()` before `RUN_ALL_TESTS()`
  - Context will be cleaned up automatically when program exits

  **Code changes**:
  ```cpp
  int main(int argc, char** argv) {
      // ... existing argument parsing ...
      
      // Initialize GL context for GL backend tests
      #if VG_LITE_RENDER_BACKEND == 2
      if (!vg_lite_test::GLContextManager::instance().initialize()) {
          std::cerr << "Warning: Failed to create GL context" << std::endl;
      }
      #endif
      
      ::testing::InitGoogleTest(&argc, argv);
      return RUN_ALL_TESTS();
  }
  ```

  **References**:
  - `tests/test_main.cpp:18-35` - Current main() function

  **Acceptance Criteria**:
  - [ ] GL context created before tests run
  - [ ] No crashes during initialization

- [ ] 3. Update test_draw_vector.cpp to remove skip logic

  **What to do**:
  - Modify `setup_render_buffer()` to not check `shouldSkipRenderingTests()`
  - Remove GTEST_SKIP() calls from DrawLines and DrawShapes tests
  - Tests should now run normally on GL backend

  **References**:
  - `tests/test_draw_vector.cpp:71-86` - Current setup_render_buffer()
  - `tests/test_draw_vector.cpp:140-180` - DrawLines and DrawShapes tests

  **Acceptance Criteria**:
  - [ ] DrawLines test runs (not skipped) on GL backend
  - [ ] DrawShapes test runs (not skipped) on GL backend
  - [ ] Tests pass without SEH exceptions

- [ ] 4. Update test_draw_vector_detail.cpp to remove skip logic

  **What to do**:
  - Same as Task 3 for this file
  - Remove GTEST_SKIP() from DrawCopyPath, DrawRectPath, DrawAppendPath, DrawArcPath

  **References**:
  - `tests/test_draw_vector_detail.cpp` - Similar pattern to test_draw_vector.cpp

  **Acceptance Criteria**:
  - [ ] All path operation tests run on GL backend
  - [ ] Tests pass

- [ ] 5. Update test_golden.cpp to remove skip logic

  **What to do**:
  - Remove skip logic from golden image tests
  - VectorDrawShapes, BlendModesComparison, FillRulesComparison, MatrixTransformComparison

  **References**:
  - `tests/test_golden.cpp`

  **Acceptance Criteria**:
  - [ ] All golden tests run on GL backend
  - [ ] Tests pass

- [ ] 6. Update test_gl_backend.cpp to remove skip logic

  **What to do**:
  - Remove skip logic from UploadBuffer, UploadYUVBuffer, Integration tests
  - These tests should work now with GL context

  **References**:
  - `tests/test_gl_backend.cpp`

  **Acceptance Criteria**:
  - [ ] UploadBuffer test runs on GL backend
  - [ ] UploadYUVBuffer test runs on GL backend
  - [ ] Integration tests run on GL backend

- [ ] 7. Build and test SW backend

  **What to do**:
  ```bash
  build.bat sw Release tests
  ```

  **Acceptance Criteria**:
  - [ ] Build succeeds
  - [ ] All 64 tests pass
  - [ ] No skipped tests (except GL-specific integration tests)

- [ ] 8. Build and test GL backend

  **What to do**:
  ```bash
  build.bat gl Release tests
  ```

  **Acceptance Criteria**:
  - [ ] Build succeeds
  - [ ] All 64 tests pass
  - [ ] No skipped tests (except disabled tests)
  - [ ] No SEH exceptions

---

## Final Verification Wave

- [ ] F1. Verify SW backend: `build.bat sw Release tests` → 100% pass, no skips
- [ ] F2. Verify GL backend: `build.bat gl Release tests` → 100% pass, no skips
- [ ] F3. Check test output for "SKIPPED" messages

---

## Commit Strategy

- **Commit 1**: `test(helpers): add GLContextManager for Windows WGL`
- **Commit 2**: `test(main): initialize GL context for GL backend tests`
- **Commit 3**: `test: remove skip logic from GL backend tests`

---

## Success Criteria

### Verification Commands
```bash
# SW Backend
build.bat sw Release tests
# Expected: 64/64 tests pass

# GL Backend
build.bat gl Release tests
# Expected: 64/64 tests pass, no skips
```

### Final Checklist
- [ ] GLContextManager implemented
- [ ] GL context created before tests
- [ ] All tests run on GL backend (not skipped)
- [ ] All tests pass on both backends