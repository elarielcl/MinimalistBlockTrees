# Download and unpack sdsl at configure time
configure_file(CMakeLists.txt.in sdsl-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sdsl-download )
if(result)
    message(FATAL_ERROR "CMake step for sdsl failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sdsl-download )
if(result)
    message(FATAL_ERROR "Build step for sdsl failed: ${result}")
endif()


# Add sdsl directly to our build. This defines
# the sdsl target.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/sdsl-src
        ${CMAKE_CURRENT_BINARY_DIR}/sdsl-build
        EXCLUDE_FROM_ALL)
