file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_library(Mirror STATIC ${SOURCES})

target_include_directories(Mirror PUBLIC src PRIVATE ${CMAKE_SOURCE_DIR}/vendor)

target_include_directories(Mirror PRIVATE ${CMAKE_SOURCE_DIR}/vendor/SDL3/include)
target_link_directories(Mirror PRIVATE ${CMAKE_SOURCE_DIR}/vendor/SDL3/lib/x64)
target_link_libraries(Mirror PRIVATE SDL3 Vulkan::Vulkan)

