set(project_EXECUTABLES
        main)


foreach( exe ${project_EXECUTABLES} )
    add_executable( ${exe} ${exe}.cpp)
endforeach()


foreach( exe ${project_EXECUTABLES} )
    target_link_libraries( ${exe} blocktree )
    target_link_libraries( ${exe} sdsl )
endforeach()