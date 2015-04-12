cmake_minimum_required ( VERSION 2.8 )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Werror -std=c11 -D_GNU_SOURCE -lglib -I/usr/include/glib-1.2 -I/usr/lib/glib/include ")

include_directories(
	${PROJECT_SOURCE_DIR}/include/core
)

add_subdirectory( ${PROJECT_SOURCE_DIR}/core/entity )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/util )
add_subdirectory( ${PROJECT_SOURCE_DIR}/tests )

		
