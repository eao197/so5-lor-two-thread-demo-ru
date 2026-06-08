cmake_minimum_required(VERSION 3.10)

set(PRJ two_thread_demo_3_app)

project(${PRJ})

add_executable(${PRJ} main.cpp)
target_link_libraries(${PRJ} sobjectizer::StaticLib)
if(WIN32)
	set_property(TARGET ${PRJ} PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

install(
	TARGETS ${PRJ}
	RUNTIME DESTINATION bin
)

