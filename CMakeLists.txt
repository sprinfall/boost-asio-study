cmake_minimum_required(VERSION 3.0)
project(boost_asio_study)

option(ENABLE_WX_ASIO_EXAMPLE "Enable the example using asio in wxWidgets?" OFF)

add_definitions(-DUNICODE -D_UNICODE)

if(MSVC)
    # Win7 (asio needs this)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

if(ENABLE_WX_ASIO_EXAMPLE)
    find_package(wxWidgets REQUIRED COMPONENTS core base)
    include(${wxWidgets_USE_FILE})
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(src)
