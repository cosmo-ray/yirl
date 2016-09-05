set (SOURCE util.c debug.c block-array.c)

add_library ( util STATIC ${SOURCE} )

target_link_libraries(
  util
  numa
)
