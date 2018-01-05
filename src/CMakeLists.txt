if(UNIX)
    add_definitions(-D_GLIBCXX_USE_WCHAR_T -std=c++11)
endif()

add_executable(empty empty.cpp)

add_executable(timer1_sync timer1_sync.cpp)
add_executable(timer2_async timer2_async.cpp)
add_executable(timer3_args timer3_args.cpp)
add_executable(timer4_memfunc timer4_memfunc.cpp)

add_executable(strand strand.cpp)

add_executable(echo_server_sync echo_server_sync.cpp)
add_executable(echo_server_sync_thread echo_server_sync_thread.cpp)
add_executable(echo_server_async echo_server_async.cpp)
add_executable(echo_client_sync echo_client_sync.cpp)
add_executable(echo_client_async echo_client_async.cpp)

add_executable(daytime_tcp_client1 daytime_tcp_client1.cpp)
add_executable(daytime_tcp_client2 daytime_tcp_client2.cpp)
add_executable(daytime_udp_client daytime_udp_client.cpp)
add_executable(daytime_tcp_server1 daytime_tcp_server1.cpp)
add_executable(daytime_tcp_server2 daytime_tcp_server2.cpp)
add_executable(daytime_udp_server1 daytime_udp_server1.cpp)
add_executable(daytime_udp_server2 daytime_udp_server2.cpp)

add_executable(alloc_server alloc_server.cpp)
add_executable(alloc_client alloc_client.cpp)

add_executable(thread_pool thread_pool.cpp)

add_executable(web_client web_client.cpp)

file(GLOB HTTP_SERVER_SRCS http/server/*.cpp http/server/*.hpp)
add_executable(http_server ${HTTP_SERVER_SRCS})

if(ENABLE_WX_ASIO_EXAMPLE)
    add_executable(wx_asio_server WIN32 MACOSX_BUNDLE wx-asio/server.cpp)
    add_executable(wx_asio_client wx-asio/client.cpp)
    target_link_libraries(wx_asio_server ${wxWidgets_LIBRARIES})
    target_link_libraries(wx_asio_client ${wxWidgets_LIBRARIES})
endif()

