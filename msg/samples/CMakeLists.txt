add_executable(basic basic.cc)
target_link_libraries(basic msg)

add_executable(server server.cc)
target_link_libraries(server msg)

add_executable(client client.cc)
target_link_libraries(client msg)
