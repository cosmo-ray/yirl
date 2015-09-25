add_definitions( -DTESTS_PATH="${CMAKE_SOURCE_DIR}/tests" )

set (SOURCE main.c lifecycle.c setunset.c lua-script.c
    test-json-desc.c test-widgets.c menu.c map.c
    test-game.c)

add_executable(
	tests
	${SOURCE}
)



if( WITH_CURSES)
  set (RENDER_LIBS ${RENDER_LIBS} wid-curses)
  add_definitions(-DWITH_CURSES=1)
endif()

if( WITH_SDL)
  set (RENDER_LIBS ${RENDER_LIBS} wid-sdl2)
  add_definitions(-DWITH_SDL=1)
endif()

message("rendre: ${RENDER_LIBS}")

target_link_libraries(
	tests
	${RENDER_LIBS}
	glib-2.0
	json-c
	entity
	util
	yes
	game
	luaScript
	jsonDescrption
	widgets
)
