cmake_minimum_required(VERSION 3.30)

project(MirrorEngine VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(mirror)
add_subdirectory(app)
add_subdirectory(test)

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT App)
endif()

