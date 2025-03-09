file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_library(Frame STATIC ${SOURCES})

target_include_directories(Frame PUBLIC src ${CMAKE_SOURCE_DIR}/vendor ${CMAKE_SOURCE_DIR}/vendor/SDL3/include)

target_link_directories(Frame PUBLIC ${CMAKE_SOURCE_DIR}/vendor/SDL3/lib/x64)
find_package(Vulkan REQUIRED)

target_link_libraries(Frame PUBLIC SDL3 Vulkan::Vulkan)

