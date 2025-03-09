file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_library(Mirror STATIC ${SOURCES})

target_include_directories(Mirror PUBLIC src)
target_link_libraries(Mirror PUBLIC Frame Reflect)

