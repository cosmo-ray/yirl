set (SOURCE widget.c
    widget-callback.c
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

add_subdirectory( ${PROJECT_SOURCE_DIR}/core/widget/curses )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/widget/sdl2 )
