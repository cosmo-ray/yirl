set (SOURCE description.c)

add_library (jsonDescrption STATIC ${SOURCE})

target_link_libraries(
	jsonDescrption
	glib-2.0
	entity
	util
)
