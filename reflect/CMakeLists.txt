file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_library(Reflect STATIC ${SOURCES})

target_include_directories(Reflect PUBLIC src)
target_link_libraries(Reflect PUBLIC Frame)

