set (SOURCE main.c)

add_executable(
	tests
	${SOURCE}
)

target_link_libraries(
	tests
	entity
	util
)
