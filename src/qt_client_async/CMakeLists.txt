# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)

set(SRCS
	daytime_client.cpp
	daytime_client.h
	main.cpp
	my_window.cpp
	my_window.h)

add_executable(qt_client_async WIN32 MACOSX_BUNDLE ${SRCS})

target_link_libraries(qt_client_async Qt5::Widgets)
