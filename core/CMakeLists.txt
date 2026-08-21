
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(puzzle_core STATIC ${SOURCES})

target_include_directories(puzzle_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}                
    ${CMAKE_SOURCE_DIR}                       
    ${CMAKE_SOURCE_DIR}/external/glad/include
)


target_link_libraries(puzzle_core PUBLIC 
    glm::glm
)