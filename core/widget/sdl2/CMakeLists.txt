set (SOURCE sdl.c text-screen.c menu.c map.c)

add_library (wid-sdl2 STATIC ${SOURCE})

target_link_libraries(
	wid-sdl2
	widgets
	SDL2
	SDL2_ttf
	SDL2_image
)
