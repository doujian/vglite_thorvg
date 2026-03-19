# Tests CMakeLists.txt
# Google Test framework integration

# Use local googletest from third_party instead of FetchContent
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${VGLITE_ROOT_DIR}/third_party/googletest ${CMAKE_BINARY_DIR}/googletest)

enable_testing()

# Test sources (C++)
file(GLOB TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_impl.cpp
)

# Create test executable if there are test sources
if(TEST_SOURCES)
    add_executable(vglite_tests ${TEST_SOURCES})
    target_link_libraries(vglite_tests PRIVATE 
        vglite_render 
        GTest::gtest
    )
    target_include_directories(vglite_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/stb
        ${VGLITE_ROOT_DIR}/include
        ${VGLITE_ROOT_DIR}/third_party/thorvg
    )
    
    # Copy golden images to build directory for tests
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ref_imgs 
         DESTINATION ${CMAKE_BINARY_DIR}/tests)
    
    # Copy SVG test assets to build directory for tests
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
         DESTINATION ${CMAKE_BINARY_DIR}/tests)
    
    # Register with CTest
    include(GoogleTest)
    gtest_discover_tests(vglite_tests)
endif()

# Create a placeholder test if no test sources exist
if(NOT TEST_SOURCES)
    add_executable(vglite_tests test_main.cpp)
    target_link_libraries(vglite_tests PRIVATE 
        vglite_render 
        GTest::gtest
    )
    target_include_directories(vglite_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${VGLITE_ROOT_DIR}/include
        ${VGLITE_ROOT_DIR}/third_party/thorvg
    )
    
    include(GoogleTest)
    gtest_discover_tests(vglite_tests)
endif()
