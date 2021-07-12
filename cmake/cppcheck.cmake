find_program(CMAKE_C_CPPCHECK NAMES cppcheck)
if (CMAKE_C_CPPCHECK)
    list(
        APPEND CMAKE_C_CPPCHECK 
            "--enable=warning"
            "--inconclusive"
            "--force" 
            "--inline-suppr"
    )
endif()
