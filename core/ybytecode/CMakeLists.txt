set (SOURCE ybytecode.c condition.c ../script/ybytecode-script.c)

add_library (ybytecode ${SOURCE})

target_link_libraries(
  ybytecode
  entity
)
