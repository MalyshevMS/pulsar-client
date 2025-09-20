cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(PROJECT_NAME pulsar)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}
    src/main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)

add_subdirectory(external/sfml sfml)
target_link_libraries(${PROJECT_NAME} sfml-network)
target_link_libraries(${PROJECT_NAME} sfml-graphics)