file(GLOB MAIN_SOURCES "*.cpp" "*.h")

# Generar bin

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    puzzle_editor
    puzzle_classes
    puzzle_globals
    puzzle_script
    puzzle_modules
    puzzle_platform
    puzzle_core
    glad
    glfw
    OpenGL::GL
)