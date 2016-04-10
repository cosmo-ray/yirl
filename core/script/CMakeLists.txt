set (LUA_SOURCE
	lua-script.c
	entity-script.c
	lua-binding.c
	lua-convert.c
	script.c)

set (TCC_SOURCE
	entity-script.c
        tcc-script.c
	script.c)

add_library (luaScript STATIC ${LUA_SOURCE})
add_library (tccScript STATIC ${TCC_SOURCE})

target_link_libraries(
	luaScript
	lua5.2
	entity
)

target_link_libraries(
	tccScript
	dl
	tcc
	entity
)
