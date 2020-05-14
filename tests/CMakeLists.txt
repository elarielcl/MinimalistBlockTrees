include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(check_gtest GTestFixture.cpp)

add_subdirectory(googletest)


target_link_libraries(check_gtest gtest gtest_main)
target_link_libraries(check_gtest blocktree)
target_link_libraries(check_gtest ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libsdsl.a)
target_link_libraries(check_gtest ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort.a)
target_link_libraries(check_gtest ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort64.a)
