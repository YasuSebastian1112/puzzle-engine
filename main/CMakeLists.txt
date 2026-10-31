file(GLOB MAIN_SOURCES "*.cpp" "*.h")


if(WIN32)
    list(APPEND MAIN_SOURCES "${CMAKE_SOURCE_DIR}/platform/windows/puzzle_engine.rc")
endif()
# Generar bin

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
)

find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    puzzle_editor
    puzzle_globals
    puzzle_core
    Threads::Threads
    puzzle_classes
    puzzle_script
    puzzle_modules
    puzzle_platform
    glad
    glfw
    OpenGL::GL
)

