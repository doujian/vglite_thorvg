# Examples CMakeLists.txt

# STB implementation for dump functionality
set(STB_IMPL_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/stb_impl.cpp)

file(GLOB EXAMPLE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE} ${STB_IMPL_SOURCE})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE vglite_render)
    target_include_directories(${EXAMPLE_NAME} PRIVATE
        ${VGLITE_ROOT_DIR}/include
        ${CMAKE_SOURCE_DIR}/tests/stb
    )
    
    # Link platform-specific libraries for OpenGL examples
    if(VGLITE_BACKEND_OPENGL)
        if(WIN32)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE opengl32)
        elseif(APPLE)
            find_package(OpenGL REQUIRED)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE OpenGL::GL)
        elseif(UNIX AND NOT ANDROID)
            find_package(OpenGL REQUIRED)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE OpenGL::GL)
        endif()
    endif()
    
    # Link ThorVG if available
    if(VGLITE_BACKEND_THORVG AND TARGET thorvg)
        target_link_libraries(${EXAMPLE_NAME} PRIVATE thorvg)
    endif()
endforeach()

# Windows-specific examples
if(WIN32)
    add_subdirectory(windows)
endif()
