cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(PROJECT_NAME pulsar-client)
project(${PROJECT_NAME} LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(WIN32)
    set(SFML_STATIC_LIBRARIES TRUE)
    set(SFML_USE_STATIC_STD_LIBS TRUE)
    add_subdirectory(external/sfml sfml)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SFML_STATIC)

    target_compile_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)
    target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)

    target_compile_definitions(${PROJECT_NAME} PRIVATE -DNO_PTHREAD)
else()
    set(SFML_STATIC_LIBRARIES FALSE)
    add_subdirectory(external/sfml sfml)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
    sfml-network
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/res"
    "${CMAKE_BINARY_DIR}/bin/res"
)

function(copy_runtime_deps TARGET_NAME)
    set(DEST_DIR "${CMAKE_BINARY_DIR}/bin/libs")

    if(UNIX AND NOT APPLE)
        message(STATUS "Copying runtime .so for Linux")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
        )
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -DINPUT=$<TARGET_FILE:${TARGET_NAME}> -DOUTPUT=${DEST_DIR}
                    -P ${CMAKE_SOURCE_DIR}/cmake/copy_linux_deps.cmake
        )
        set_target_properties(${TARGET_NAME} PROPERTIES
            BUILD_RPATH "\$ORIGIN/libs"
            INSTALL_RPATH "\$ORIGIN/libs"
        )
    endif()
endfunction()

copy_runtime_deps(${PROJECT_NAME})