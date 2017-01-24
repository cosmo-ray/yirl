set (SOURCE description.c json-desc.c rawfile-decs.c)

add_library (jsonDescrption STATIC ${SOURCE})

target_link_libraries(
	jsonDescrption
	entity
)
