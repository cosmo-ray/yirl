set (SOURCE widget.c
    widget-callback.c
    text-screen.c
    menu.c
    map.c
    pos.c
    rect.c
    contener.c)

add_library (widgets STATIC ${SOURCE})

target_link_libraries(
	widgets
	entity
)

if( WITH_CURSES)
  message( "-- Option WITH_CURSES is enable" )
  add_subdirectory( ${PROJECT_SOURCE_DIR}/core/widget/curses )
endif()
if( WITH_SDL)
  message( "-- Option WITH_SDL is enable" )
  add_subdirectory( ${PROJECT_SOURCE_DIR}/core/widget/sdl2 )
endif()
