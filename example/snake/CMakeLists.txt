add_definitions( -DTESTS_PATH="${CMAKE_SOURCE_DIR}/example/snake" )

set (SOURCE main.c)

add_executable(
	snake
	${SOURCE}
)

target_link_libraries(
	snake
	game
)
