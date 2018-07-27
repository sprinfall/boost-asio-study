cmake_minimum_required(VERSION 3.0)
project(boost_asio_study)

option(ENABLE_SSL "Enable SSL/HTTPS examples (need OpenSSL)?" ON)

# See: https://stackoverflow.com/a/40217291
if(WIN32)
	macro(get_WIN32_WINNT version)
		if(CMAKE_SYSTEM_VERSION)
			set(ver ${CMAKE_SYSTEM_VERSION})
			string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
			string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
			# Check for Windows 10, b/c we'll need to convert to hex 'A'.
			if("${verMajor}" MATCHES "10")
				set(verMajor "A")
				string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
			endif("${verMajor}" MATCHES "10")
			# Remove all remaining '.' characters.
			string(REPLACE "." "" ver ${ver})
			# Prepend each digit with a zero.
			string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
			set(${version} "0x${ver}")
		endif(CMAKE_SYSTEM_VERSION)
	endmacro(get_WIN32_WINNT)

	get_WIN32_WINNT(ver)
	# E.g., 0x0601 for Win7.
	message(STATUS "_WIN32_WINNT=${ver}")
    # Asio needs this!
	add_definitions(-D_WIN32_WINNT=${ver})
endif(WIN32)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

if(ENABLE_SSL)
	set(OPENSSL_USE_STATIC_LIBS ON)
	set(OPENSSL_MSVC_STATIC_RT ON)
	find_package(OpenSSL)
	if(OPENSSL_FOUND)
		include_directories(${OPENSSL_INCLUDE_DIR})
	endif()
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(src)
