set (SOURCE entity.c)

add_library (entity STATIC ${SOURCE})

target_link_libraries(
	entity
	util
)
