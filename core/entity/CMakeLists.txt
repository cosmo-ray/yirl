set (SOURCE entity.c entity-convertions.c)

add_library (entity STATIC ${SOURCE})

target_link_libraries(
	entity
	glib-2.0
	util
)
