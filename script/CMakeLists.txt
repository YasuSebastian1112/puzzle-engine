file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
add_library(puzzle_script STATIC ${SOURCES})

target_include_directories(puzzle_script PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(puzzle_script PUBLIC puzzle_core)