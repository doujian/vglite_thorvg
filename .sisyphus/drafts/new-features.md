# Draft: New Features

## Requirements (confirmed)

### 1. Unit Test Dump Parameter
- **Method**: Command line parameter
- **Example**: `--dump` or `--output-dir=path`
- **Behavior**: When enabled, dump test output images/data after test execution

### 2. Android OpenGL ES Build Support
- **Build System**: CMake + NDK
- **Backend**: OpenGL ES (not SW)
- **ABI**: arm64-v8a only

### 3. Examples with vg_lite_clear
- **Windows**: Create window, use vg_lite_clear, display until user closes
- **Android**: Same behavior on Android
- **Window Size**: Fullscreen
- **Output Spec**: Need to clarify clear color and image format

## Open Questions

- [ ] Clear color for vg_lite_clear example?
- [ ] What image format to display? (solid color? pattern?)
- [ ] Dump output format for unit tests? (PNG only? or also raw data?)
- [ ] Should Android example be a full APK or just native library?

## Technical Decisions

### 1. Dump Parameter Implementation
- Use Google Test's command line argument parsing
- Add `--gtest_filter` style custom flag
- Store output path in global test config

### 2. Android Build
- Create `CMakeLists.txt` with Android NDK toolchain
- Enable `VG_LITE_USE_GL_BACKEND` option
- Provide build instructions

### 3. Examples
- Windows: Use Win32 API or GLFW for window management
- Android: Use NativeActivity for window
- Both use vg_lite_clear with fullscreen buffer