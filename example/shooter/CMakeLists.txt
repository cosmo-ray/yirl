add_definitions( -DTESTS_PATH="${CMAKE_SOURCE_DIR}/example/snake" )

set (SOURCE main.c)

add_executable(
	shooter
	${SOURCE}
)

target_link_libraries(
	shooter
	game
)
