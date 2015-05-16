set (SOURCE lua-script.c
    	    script.c)

add_library (luaScript STATIC ${SOURCE})

target_link_libraries(
	luaScript
	glib-2.0
	lua
	entity
	util
)
