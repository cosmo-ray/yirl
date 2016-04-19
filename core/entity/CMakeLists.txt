set (SOURCE entity.c)

add_library (entity STATIC ${SOURCE})

target_link_libraries(
	entity
	glib-2.0
	util
)
