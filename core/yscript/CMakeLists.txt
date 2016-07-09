set (SOURCE yscript-bytecode.c)

add_executable(yscript ${SOURCE})

target_link_libraries(
  yscript
  entity
)
