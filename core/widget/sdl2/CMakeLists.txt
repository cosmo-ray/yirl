set (SOURCE sdl.c text-screen.c  menu.c)

add_library (wid-sdl2 STATIC ${SOURCE})

target_link_libraries(
	wid-sdl2
	glib-2.0
	entity
	util
	widgets	
	SDL2
	SDL2_ttf
	SDL2_image
)
