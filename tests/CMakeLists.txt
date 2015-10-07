add_definitions( -DTESTS_PATH="${CMAKE_SOURCE_DIR}/tests" )

set (SOURCE main.c lifecycle.c setunset.c lua-script.c
    test-json-desc.c test-widgets.c menu.c map.c
    test-game.c)

add_executable(
	tests
	${SOURCE}
)

target_link_libraries(
	tests
	glib-2.0
	json-c
	entity
	util
	yes
	game
	luaScript
	jsonDescrption
	widgets
	wid-curses
	wid-sdl2
)
