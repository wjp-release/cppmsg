add_executable(basic basic.cc)
target_link_libraries(basic msg)

add_executable(server server.cc)
target_link_libraries(server msg)

add_executable(client client.cc)
target_link_libraries(client msg)

add_executable(wjpperf perf.cc)
target_link_libraries(wjpperf msg)

add_executable(arenaperf arenaperf.cc)
target_link_libraries(arenaperf msg)


