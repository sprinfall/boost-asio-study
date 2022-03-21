set(UTILITY_SRCS utility.cpp utility.h)

set(LIBS ${Boost_LIBRARIES} "${CMAKE_THREAD_LIBS_INIT}")

if(ENABLE_SSL)
	set(LIBS ${LIBS} ${OPENSSL_LIBRARIES})
	if(WIN32)
		set(LIBS ${LIBS} crypt32)
	endif()
endif()

if(UNIX)
	# Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
	set(LIBS ${LIBS} ${CMAKE_DL_LIBS})
endif()

set(TARGETS
    hello
    timer1_sync
    timer2_async
    timer3_lambda
    timer4_multi
    timer5_threaded
    timer6_args
    timer7_memfunc
    strand
    strand2
    echo_server_sync
    echo_server_async
    echo_client_sync
    echo_client_async
    
    context_and_services
    )

if(ENABLE_SSL)
	set(TARGETS ${TARGETS}
		ssl_http_client_sync
		ssl_http_client_async
		ssl_http_client_async_blocking
		ssl_http_client_async_blocking_timeout
		)
endif()

foreach(name ${TARGETS})
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} ${LIBS})
endforeach()

if(ENABLE_QT)
	add_subdirectory(qt_client_async)
endif()
