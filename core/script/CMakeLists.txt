set (LUA_SOURCE
	lua-script.c
	lua-convert.c
	script.c)

set (TCC_SOURCE
        tcc-script.c
	script.c)

add_library (luaScript STATIC ${LUA_SOURCE})
add_library (tccScript STATIC ${TCC_SOURCE})

target_link_libraries(
	luaScript
	glib-2.0
	lua
	entity
	util
)

target_link_libraries(
	tccScript
	glib-2.0
	tcc
	entity
	util
)
