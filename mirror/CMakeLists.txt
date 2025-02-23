file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_library(Mirror STATIC ${SOURCES})

target_include_directories(Mirror PUBLIC src ${CMAKE_SOURCE_DIR}/vendor ${CMAKE_SOURCE_DIR}/vendor/SDL3/include)
target_link_directories(Mirror PUBLIC ${CMAKE_SOURCE_DIR}/vendor/SDL3/lib/x64)
find_package(Vulkan REQUIRED)

target_link_libraries(Mirror PUBLIC SDL3 Vulkan::Vulkan)

