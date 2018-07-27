set(UTILITY_SRCS utility.cpp utility.h)

if(ENABLE_SSL)
	set(SSL_LIBS ${OPENSSL_LIBRARIES})
	if(WIN32)
		set(SSL_LIBS ${SSL_LIBS} crypt32)
	endif()
endif()

add_executable(hello hello.cpp)

add_executable(timer1_sync timer1_sync.cpp)
add_executable(timer2_async timer2_async.cpp)
add_executable(timer3_args timer3_args.cpp)
add_executable(timer4_memfunc timer4_memfunc.cpp)

add_executable(strand strand.cpp)

add_executable(echo_server_sync echo_server_sync.cpp)
add_executable(echo_server_sync_thread echo_server_sync_thread.cpp)
add_executable(echo_server_async echo_server_async.cpp)
add_executable(echo_client_sync echo_client_sync.cpp)
add_executable(echo_client_async echo_client_async.cpp )

if(ENABLE_SSL)
	add_executable(ssl_http_client_sync ssl_http_client_sync.cpp)
	add_executable(ssl_http_client_async ssl_http_client_async.cpp)
	add_executable(ssl_http_client_async_blocking ssl_http_client_async_blocking.cpp)

	target_link_libraries(ssl_http_client_sync ${SSL_LIBS})
	target_link_libraries(ssl_http_client_async ${SSL_LIBS})
	target_link_libraries(ssl_http_client_async_blocking ${SSL_LIBS})
endif()