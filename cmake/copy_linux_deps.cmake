if(UNIX AND NOT MINGW)
    if(NOT DEFINED INPUT)
        message(FATAL_ERROR "INPUT not set")
    endif()
    if(NOT DEFINED OUTPUT)
        message(FATAL_ERROR "OUTPUT not set")
    endif()

    execute_process(
        COMMAND ldd "${INPUT}"
        OUTPUT_VARIABLE LDD_OUTPUT
        ERROR_VARIABLE LDD_ERROR
        RESULT_VARIABLE LDD_RESULT
    )

    if(NOT LDD_RESULT EQUAL 0)
        message(FATAL_ERROR "ldd failed: ${LDD_ERROR}")
    endif()

    string(REPLACE "\n" ";" LDD_LINES "${LDD_OUTPUT}")

    foreach(line ${LDD_LINES})
        if(line MATCHES "=> (/[^ ]+)")
            set(LIB "${CMAKE_MATCH_1}")
            if(EXISTS "${LIB}")
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LIB}" "${OUTPUT}"
                )
            endif()
        endif()
    endforeach()
endif()
