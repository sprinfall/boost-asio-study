set(UTILITY_SRCS utility.cpp utility.h)

set(STD_LIBS ${Boost_LIBRARIES} "${CMAKE_THREAD_LIBS_INIT}")

if(ENABLE_SSL)
	set(SSL_LIBS ${OPENSSL_LIBRARIES})
	if(WIN32)
		set(SSL_LIBS ${SSL_LIBS} crypt32)
	else()
		# Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
		set(SSL_LIBS ${OPENSSL_LIBRARIES} ${CMAKE_DL_LIBS})
	endif()
endif()

add_executable(hello hello.cpp)
target_link_libraries(hello ${STD_LIBS})

add_executable(timer1_sync timer1_sync.cpp)
add_executable(timer2_async timer2_async.cpp)
add_executable(timer2_async_lambda timer2_async_lambda.cpp)
add_executable(timer3_args timer3_args.cpp)
add_executable(timer4_memfunc timer4_memfunc.cpp)

target_link_libraries(timer1_sync ${STD_LIBS})
target_link_libraries(timer2_async ${STD_LIBS})
target_link_libraries(timer2_async_lambda ${STD_LIBS})
target_link_libraries(timer3_args ${STD_LIBS})
target_link_libraries(timer4_memfunc ${STD_LIBS})

add_executable(strand strand.cpp)
target_link_libraries(strand ${STD_LIBS})

add_executable(echo_server_sync echo_server_sync.cpp)
add_executable(echo_server_async echo_server_async.cpp)
add_executable(echo_client_sync echo_client_sync.cpp)
add_executable(echo_client_async echo_client_async.cpp ${UTILITY_SRCS})

target_link_libraries(echo_server_sync ${STD_LIBS})
target_link_libraries(echo_server_async ${STD_LIBS})
target_link_libraries(echo_client_sync ${STD_LIBS})
target_link_libraries(echo_client_async ${STD_LIBS})

if(ENABLE_SSL)
	add_executable(ssl_http_client_sync ssl_http_client_sync.cpp)
	add_executable(ssl_http_client_async ssl_http_client_async.cpp)
	add_executable(ssl_http_client_async_blocking ssl_http_client_async_blocking.cpp)
	# add_executable(ssl_http_client_async_blocking_timeout ssl_http_client_async_blocking_timeout.cpp)

	target_link_libraries(ssl_http_client_sync ${STD_LIBS} ${SSL_LIBS})
	target_link_libraries(ssl_http_client_async ${STD_LIBS} ${SSL_LIBS})
	target_link_libraries(ssl_http_client_async_blocking ${STD_LIBS} ${SSL_LIBS})
	# target_link_libraries(ssl_http_client_async_blocking_timeout ${STD_LIBS} ${SSL_LIBS})
endif()

if(ENABLE_QT)
	add_subdirectory(qt_client_async)
endif()
