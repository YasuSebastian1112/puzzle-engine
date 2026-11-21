file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
add_library(puzzle_editor STATIC ${SOURCES})

target_include_directories(puzzle_editor PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(puzzle_editor PUBLIC 
    puzzle_classes 
    puzzle_globals 
    puzzle_script
)