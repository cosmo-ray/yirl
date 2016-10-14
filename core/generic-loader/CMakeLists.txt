set (SOURCE main.c)

add_executable(
	yirl-loader
	${SOURCE}
)

target_link_libraries(
	yirl-loader
	game
)
