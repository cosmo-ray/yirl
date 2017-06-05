#TODO: use an object lib for script.c and native-script.c

set (LUA_SOURCE
	lua-script.c
	entity-script.c
	lua-binding.c
	lua-convert.c
	script.c)

set (TCC_SOURCE
	entity-script.c
        tcc-script.c
	native-script.c
	ybytecode-script.c
	script.c)

add_library (luaScript STATIC ${LUA_SOURCE})
add_library (tccScript STATIC ${TCC_SOURCE})

target_link_libraries(
	luaScript
	widgets
	${LUA_LIB_PATH}
	entity
)

message("tcc path: " ${TCC_LIB_PATH})

target_link_libraries(
	tccScript
	dl
	ybytecode
	${TCC_LIB_PATH}/libtcc.a
	entity
)
