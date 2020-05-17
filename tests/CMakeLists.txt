include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_subdirectory(compressed)
add_subdirectory(pointer_based)

add_subdirectory(googletest)
