file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
add_library(puzzle_platform STATIC ${SOURCES})

target_include_directories(puzzle_platform PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(puzzle_platform PUBLIC puzzle_core glfw)