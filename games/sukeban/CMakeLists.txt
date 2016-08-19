add_definitions( -DSUKEBAN_PATH="${CMAKE_SOURCE_DIR}/games/sukeban" )

set (SOURCE main.c)

add_executable(
	sukeban
	${SOURCE}
)

target_link_libraries(
	sukeban
	game
)
