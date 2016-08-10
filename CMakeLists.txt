cmake_minimum_required(VERSION 2.8)
project(boost_asio_study)

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

# Add target property.
# Usage: ADD_TARGET_PROPERTY(target COMPILE_FLAGS "/we1234")
macro(ADD_TARGET_PROPERTY target_name property_key property_value)
    get_target_property(CTS_COMPILE_FLAGS ${target_name} ${property_key} )
    #	message("t:${target_name} k:${property_key} v:${property_value} ${CTS_COMPILE_FLAGS}")
    if(CTS_COMPILE_FLAGS STREQUAL CTS_COMPILE_FLAGS-NOTFOUND)
        SET_TARGET_PROPERTIES(${target_name} PROPERTIES
            ${property_key} ${property_value})
    else()
        SET_TARGET_PROPERTIES(${target_name} PROPERTIES
            ${property_key} "${CTS_COMPILE_FLAGS} ${property_value}")
    endif()
endmacro()

# Common target setting.
# Usage: COMMON_TARGET_SETTING(target)
macro(COMMON_TARGET_SETTING target_name)
    if(WIN32)
        if(MSVC)
            # treat c4239 c4482 c4715 as error, as these can cause error on gcc
            # disable c4407 c4290
            ADD_TARGET_PROPERTY(${target_name} COMPILE_FLAGS "/we4239 /we4482 /we4715 /wd4407 /wd4290")
        endif()
    endif()
endmacro()

include_directories(${PROJECT_SOURCE_DIR}/src)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(src)
