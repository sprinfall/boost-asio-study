if(UNIX)
    add_definitions(-D_GLIBCXX_USE_WCHAR_T -std=c++11)
endif()

add_executable(empty empty.cpp)

add_executable(timer1 timer1.cpp)
add_executable(timer2 timer2.cpp)
add_executable(timer3 timer3.cpp)
add_executable(timer4 timer4.cpp)
add_executable(timer5 timer5.cpp)

add_executable(strand strand.cpp)

add_executable(echo_server1 echo_server1.cpp)
add_executable(echo_server2 echo_server2.cpp)
add_executable(echo_client1 echo_client1.cpp)
add_executable(echo_client2 echo_client2.cpp)

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

add_executable(uv_hello uv_hello.cpp)
add_executable(uv_idle_basic uv_idle_basic.cpp)
