# VGLite ThorVG

VGLite-compatible rendering library with ThorVG backend support.

## Features

- **Dual Backend Support**: Software (SW) and OpenGL (GL) rendering backends
- **Cross-Platform**: Windows and Android support
- **VGLite API Compatible**: Drop-in replacement for VGLite API
- **ThorVG Integration**: High-quality vector graphics rendering

## Project Structure

```
vglite_thorvg/
├── src/                    # VGLite implementation
├── include/                # Public headers
├── third_party/            # ThorVG library
├── tests/                  # Unit tests
├── examples/
│   ├── windows/            # Windows example (hello_window)
│   └── android/            # Android example app
└── docs/                   # Documentation
```

## Building

### Windows

**Requirements:**
- Visual Studio 2019+
- CMake 3.16+

**Build SW Backend:**
```bash
cmake -B build/windows/sw -G "Visual Studio 17 2022" -A x64
cmake --build build/windows/sw --config Release
```

**Build GL Backend:**
```bash
cmake -B build/windows/gl -G "Visual Studio 17 2022" -A x64 -DVG_LITE_USE_GL_BACKEND=ON
cmake --build build/windows/gl --config Release
```

### Android

**Requirements:**
- Android Studio / NDK 29+
- CMake 3.22+

**Build APK:**
```bash
cd examples/android
./gradlew assembleDebug
```

Output: `build/android/gl/app-debug.apk`

## Running Tests

```bash
cd build/windows/sw  # or build/windows/gl
ctest -C Release --output-on-failure
```

## Output Directory Structure

Build artifacts are organized by platform and backend:

```
build/
├── android/
│   └── gl/              # Android GL backend APK
└── windows/
    ├── gl/              # Windows GL backend
    │   ├── bin/Release/ # Executables
    │   └── lib/Release/ # Libraries
    └── sw/              # Windows SW backend
        ├── bin/Release/
        └── lib/Release/
```

## License

MIT License
