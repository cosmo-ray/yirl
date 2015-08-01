set (SOURCE widget.c
    text-screen.c
    menu.c
    map.c)

add_library (widgets STATIC ${SOURCE})

target_link_libraries(
	widgets
	glib-2.0
	entity
	util
)

add_subdirectory( ${PROJECT_SOURCE_DIR}/widget/curses )
add_subdirectory( ${PROJECT_SOURCE_DIR}/widget/sdl2 )
