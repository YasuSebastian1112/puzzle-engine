file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
add_library(puzzle_modules STATIC ${SOURCES})

target_include_directories(puzzle_modules PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(puzzle_modules PUBLIC puzzle_core)