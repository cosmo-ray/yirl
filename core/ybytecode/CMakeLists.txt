set (SOURCE ybytecode.c)

add_library (ybytecode ${SOURCE})

target_link_libraries(
  ybytecode
  entity
)
