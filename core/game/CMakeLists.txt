set (SOURCE game.c)

add_library (game STATIC ${SOURCE})

target_link_libraries(
	game
	glib-2.0
	json-c
	entity
	util
	luaScript
	jsonDescrption
	widgets
	wid-curses
	wid-sdl2
)
