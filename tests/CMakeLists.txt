set (SOURCE main.c lifecycle.c)

add_executable(
	tests
	${SOURCE}
)

target_link_libraries(
	tests
	glib-2.0
	entity
	util
)
