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

if( WITH_CURSES)
  message( "i have l'option WITH_CURSES" )
  add_subdirectory( ${PROJECT_SOURCE_DIR}/core/widget/curses )
endif()
if( WITH_SDL)
  message( "j'ai the option WITH_SDL" )
  add_subdirectory( ${PROJECT_SOURCE_DIR}/core/widget/sdl2 )
endif()