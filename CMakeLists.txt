cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(PROJECT_NAME pulsar-client)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}
    src/main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
)

set(SFML_STATIC_LIBRARIES FALSE)
add_subdirectory(external/sfml sfml)

target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
    sfml-network
)

function(copy_runtime_deps TARGET_NAME)
    set(DEST_DIR $<TARGET_FILE_DIR:${TARGET_NAME}>/libs)

    if(WIN32)
        message(STATUS "Configuring runtime dependency copying for Windows")

        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
        )

        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${TARGET_NAME}>
            "${DEST_DIR}"
        )

    elseif(UNIX AND NOT APPLE)
        message(STATUS "Configuring runtime dependency copying for Linux")

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
