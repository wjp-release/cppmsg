aux_source_directory(./protocol protocol_srcs)
aux_source_directory(./transport transport_srcs)
aux_source_directory(./reactor reactor_srcs)
aux_source_directory(./common common_srcs)
add_library(msg ${protocol_srcs} ${transport_srcs} ${reactor_srcs} ${common_srcs})  
if(UNIX)
	target_link_libraries(msg pthread)
endif()

