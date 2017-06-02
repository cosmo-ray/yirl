set (SOURCE game.c)

add_library (game STATIC ${SOURCE})

if( WITH_CURSES)
  set (RENDER_LIBS ${RENDER_LIBS} wid-curses)
  add_definitions(-DWITH_CURSES=1)
endif()

if( WITH_SDL)
  set (RENDER_LIBS ${RENDER_LIBS} wid-sdl2)
  add_definitions(-DWITH_SDL=1)
endif()

target_link_libraries(
	game
	glib-2.0
	${RENDER_LIBS}
	json-c
	entity
	util
	widgets
	ybytecode
	luaScript
	tccScript
	jsonDescrption
        sound
)
