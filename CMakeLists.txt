cmake_minimum_required (VERSION 2.6)
project (sodiumpp)

if(NOT WIN32)
	add_definitions(
		-std=c++11 # Or -std=c++0x
	)
else()
	ADD_DEFINITIONS(
		-DSODIUM_STATIC
	)
endif()

if(SODIUMPP_STATIC)
	add_library(sodiumpp STATIC sodiumpp.cpp)
else()
	add_library(sodiumpp SHARED sodiumpp.cpp)
	target_link_libraries(sodiumpp sodium)
endif()

if(NOT SODIUMPP_STATIC)
	add_executable(example example.cpp)
	target_link_libraries(example sodiumpp)
endif()

install_files(/include .h)
install_targets(/lib sodiumpp)
