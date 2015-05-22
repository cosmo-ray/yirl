set (SOURCE entity-script.c lua-binding.c)

add_library (yes STATIC ${SOURCE})

target_link_libraries(
	yes
	glib-2.0
	lua
	entity
	util
)
