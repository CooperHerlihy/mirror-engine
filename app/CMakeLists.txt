file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "src/*.vert" "src/*.frag")
add_executable(App ${SOURCES})

target_include_directories(App PUBLIC src ${CMAKE_SOURCE_DIR}/vendor)
target_link_libraries(App PUBLIC Mirror)

add_custom_command(TARGET App POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_SOURCE_DIR}/vendor/SDL3/lib/x64/SDL3.dll
	$<TARGET_FILE_DIR:App>
)

