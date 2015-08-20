cmake_minimum_required ( VERSION 2.8 )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -g3 -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Werror -std=c11 -D_GNU_SOURCE -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include")

include_directories(
	${PROJECT_SOURCE_DIR}/include/core
	${PROJECT_SOURCE_DIR}/include/widget
)

add_subdirectory( ${PROJECT_SOURCE_DIR}/core/entity )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/util )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/script )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/description )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/entity-script )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/widget )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/game )
add_subdirectory( ${PROJECT_SOURCE_DIR}/tests )

		
