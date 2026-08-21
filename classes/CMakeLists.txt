file(GLOB_RECURSE CLASSES_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(puzzle_classes STATIC ${CLASSES_SOURCES})

target_include_directories(puzzle_classes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/stb
)

target_link_libraries(puzzle_classes PUBLIC
    glad
    glm::glm
)