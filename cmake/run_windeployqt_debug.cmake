# Runs only for Debug
if(NOT CONFIG STREQUAL "Debug")
    return()
endif()

if(NOT EXISTS "${WINDEPLOYQT}")
    message(FATAL_ERROR "windeployqt not found at: ${WINDEPLOYQT}")
endif()

execute_process(
        COMMAND "${WINDEPLOYQT}" --no-translations --no-compiler-runtime "${APP}"
        RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "windeployqt failed with code ${rc}")
endif()