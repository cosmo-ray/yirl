add_definitions( -DTESTS_PATH="${CMAKE_SOURCE_DIR}/example/modules/snake/" )

set (SOURCE main.c)

add_executable(
	snakeMod
	${SOURCE}
)

target_link_libraries(
	snakeMod
	game
)
