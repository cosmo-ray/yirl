set (SOURCE curses.c text-screen.c menu.c map.c)

add_library (wid-curses STATIC ${SOURCE})

target_link_libraries(
	wid-curses
	glib-2.0
	entity
	util
	widgets
	curses
)
