add_definitions( -DTESTS_PATH="${CMAKE_SOURCE_DIR}/tests" )

set (SOURCE main.c lifecycle.c setunset.c lua-script.c)

add_executable(
	tests
	${SOURCE}
)

target_link_libraries(
	tests
	glib-2.0
	entity
	util
	luaScript
)
