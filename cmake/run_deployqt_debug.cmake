# Runs only for Debug
if(NOT CONFIG STREQUAL "Debug")
    return()
endif()

if(NOT EXISTS "${DEPLOYQT}")
    message(FATAL_ERROR "deployqt not found at: ${DEPLOYQT}")
endif()

execute_process(
        COMMAND "${DEPLOYQT}" --debug --no-translations --no-compiler-runtime "${APP}"
        RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "deployqt failed with code ${rc}")
endif()