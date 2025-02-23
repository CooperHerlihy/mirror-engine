file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "src/*.vert" "src/*.frag")
add_executable(Test ${SOURCES})

target_include_directories(Test PUBLIC src)
target_link_libraries(Test PUBLIC Mirror)

add_custom_command(TARGET Test POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_SOURCE_DIR}/vendor/SDL3/lib/x64/SDL3.dll
	$<TARGET_FILE_DIR:Test>
)

