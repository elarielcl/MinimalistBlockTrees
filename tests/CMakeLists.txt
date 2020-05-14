include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(test_NAMES
        GTestFixture)


foreach( exe ${test_NAMES} )
    add_executable( check_${exe} ${exe}.cpp)
endforeach()

add_subdirectory(googletest)

foreach( exe ${test_NAMES} )
    target_link_libraries( check_${exe} gtest gtest_main)
    target_link_libraries( check_${exe} blocktree )
    target_link_libraries( check_${exe} ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a )
    target_link_libraries( check_${exe} ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libdivsufsort.a )
    target_link_libraries( check_${exe} ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libdivsufsort64.a )
endforeach()