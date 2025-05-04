cmake_minimum_required(VERSION 3.30)

project(MirrorEngine VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

find_package(Vulkan REQUIRED)

add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)

file(GLOB_RECURSE FRAME_SOURCES "src/frame/*.cpp" "include/frame/*.h")
add_library(Frame STATIC ${FRAME_SOURCES})
target_include_directories(Frame PUBLIC include vendor)
target_link_libraries(Frame PUBLIC Vulkan::Vulkan SDL3::SDL3)

file(GLOB_RECURSE REFLECT_SOURCES "src/reflect/*.cpp" "include/reflect/*.h")
add_library(Reflect STATIC ${REFLECT_SOURCES})
target_include_directories(Reflect PUBLIC include)
target_link_libraries(Reflect PUBLIC Frame)

file(GLOB_RECURSE MIRROR_SOURCES "src/mirror/*.cpp" "include/mirror/*.h")
add_library(Mirror STATIC ${MIRROR_SOURCES})
target_include_directories(Mirror PUBLIC include)
target_link_libraries(Mirror PUBLIC Frame Reflect)

file(GLOB_RECURSE APP_SOURCES "src/app/*.cpp" "include/app*.h" "shaders/*.vert" "shaders/*.frag")
add_executable(App ${APP_SOURCES})
target_include_directories(App PUBLIC include)
target_link_libraries(App PUBLIC Mirror)

file(GLOB_RECURSE SOURCES "src/test/*.cpp" "include/test/*.h" "shaders/*.vert" "shaders/*.frag")
add_executable(Test ${SOURCES})
target_include_directories(Test PUBLIC include)
target_link_libraries(Test PUBLIC Mirror)

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT App)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /W4")
endif()

