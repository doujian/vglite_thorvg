# Windows Examples CMakeLists.txt
# Builds Windows-specific VGLite examples

# Only build on Windows
if(NOT WIN32)
    message(STATUS "Windows examples skipped: not on Windows platform")
    return()
endif()

# STB implementation for dump functionality (shared with main examples)
set(STB_IMPL_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/../stb_impl.cpp)

# Find the source files in this directory
file(GLOB WINDOWS_EXAMPLE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Build each example
foreach(EXAMPLE_SOURCE ${WINDOWS_EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    
    add_executable(${EXAMPLE_NAME} WIN32 ${EXAMPLE_SOURCE} ${STB_IMPL_SOURCE})
    
    # Link against vg_lite library
    target_link_libraries(${EXAMPLE_NAME} PRIVATE vglite_render)
    
    # Include directories
    target_include_directories(${EXAMPLE_NAME} PRIVATE
        ${VGLITE_ROOT_DIR}/include
        ${CMAKE_SOURCE_DIR}/tests/stb
    )
    
    # Link Windows libraries
    target_link_libraries(${EXAMPLE_NAME} PRIVATE
        user32      # Win32 user interface
        gdi32       # GDI for simple rendering
    )
    
    message(STATUS "Configured Windows example: ${EXAMPLE_NAME}")
endforeach()

# Print summary
message(STATUS "Windows examples configured: ${WINDOWS_EXAMPLE_SOURCES}")