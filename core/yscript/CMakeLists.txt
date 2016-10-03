set (SOURCE yscript-bytecode.c)

add_library (yscript ${SOURCE})

target_link_libraries(
  yscript
  entity
)
