set(test_NAMES
        CBlockTreeFixture
        CBitBlockTreeFixture)


foreach( exe ${test_NAMES} )
    add_executable( check_${exe} ${exe}.cpp)
endforeach()


foreach( exe ${test_NAMES} )
    target_link_libraries( check_${exe} gtest gtest_main)
    target_link_libraries( check_${exe} blocktree )
    target_link_libraries( check_${exe} sdsl )
endforeach()