add_definitions( -DTESTS_PATH="${CMAKE_SOURCE_DIR}/example/modules/smReader/" )

set (SOURCE main.c)

add_executable(
	smReaderTest
	${SOURCE}
)

target_link_libraries(
	smReaderTest
	game
)
