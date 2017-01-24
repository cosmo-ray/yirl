cmake_minimum_required ( VERSION 2.8 )

if( NOT TCC_LIB_PATH)
  set (TCC_LIB_PATH ${PROJECT_SOURCE_DIR}/tinycc/)
  set (TCC_INTERNAL_LIB_PATH ${PROJECT_SOURCE_DIR}/tinycc/)
endif()
if ( TCC_INTERNAL_LIB_PATH )
  add_definitions( -DTCC_LIB_PATH="${TCC_INTERNAL_LIB_PATH}" )
else()
  add_definitions( -DTCC_LIB_PATH="${TCC_LIB_PATH}" )
endif()

if( NOT LUA_LIB_PATH)
  set (LUA_LIB_PATH lua)
endif()

message("TCC_LIB_PATH: " ${TCC_LIB_PATH})
message("TCC_INTERNAL_LIB_PATH: " ${TCC_INTERNAL_LIB_PATH})


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(GLIB2 REQUIRED)
 
add_definitions( -DYIRL_INCLUDE_PATH="${CMAKE_SOURCE_DIR}/include/" )

option(WITH_SDL "use SDL driver" ON)
option(WITH_CURSES "use curses driver" ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -g3 -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Werror -std=gnu11 -D_GNU_SOURCE -Ofast")

include_directories(
	${LUA_INCLUDE_DIR}
	${SDL2_INCLUDE_DIR}
	${GLIB2_INCLUDE_DIRS}
	${GLIB2_INTERNAL_INCLUDE_DIRS}
	${TCC_LIB_PATH}
	${PROJECT_SOURCE_DIR}/include/core
	${PROJECT_SOURCE_DIR}/include/widget
)

add_subdirectory( ${PROJECT_SOURCE_DIR}/core/entity )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/util )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/script )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/description )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/widget )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/game )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/sound )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/ybytecode )
add_subdirectory( ${PROJECT_SOURCE_DIR}/core/generic-loader )
add_subdirectory( ${PROJECT_SOURCE_DIR}/tests )
add_subdirectory( ${PROJECT_SOURCE_DIR}/example )
add_subdirectory( ${PROJECT_SOURCE_DIR}/games/sukeban )
