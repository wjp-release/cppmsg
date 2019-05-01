aux_source_directory(./protocol protocol_srcs)
aux_source_directory(./protocol/tasks tasks_srcs)
aux_source_directory(./system system_srcs)
aux_source_directory(./common common_srcs)
add_library(msg ${tasks_srcs} ${protocol_srcs} ${system_srcs} ${common_srcs})  
if(UNIX)
	target_link_libraries(msg pthread)
endif()

