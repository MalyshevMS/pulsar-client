# INPUT  = путь к бинарнику
# OUTPUT = каталог libs

execute_process(
    COMMAND ldd ${INPUT}
    OUTPUT_VARIABLE LDD_OUTPUT
)

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