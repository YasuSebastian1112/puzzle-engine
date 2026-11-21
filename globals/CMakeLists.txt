file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
add_library(puzzle_globals STATIC ${SOURCES})

target_include_directories(puzzle_globals PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/glad/include
)
target_link_libraries(puzzle_globals PUBLIC 
    puzzle_core
    puzzle_classes
    glad
    glfw
    OpenGL::GL
    Freetype::Freetype
)

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(puzzle_globals PRIVATE ${X11_LIBRARIES})
endif()