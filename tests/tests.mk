TESTS_NAME = yirl-tests

TESTS_SRC =  main.c lifecycle.c stringOperations.c test-copy.c setunset.c	\
		lua-script.c tcc-script.c					\
		test-json-desc.c test-widgets.c menu.c map.c			\
		test-game.c block-array.c container.c 				\
		ybytecode.c script.c raw-file.c list-mod.c dialogue-mod.c	\
		maze_generator.c canvas.c textinput_mod.c sukeban-fight.c	\
		test-sound.c dialogue-box.c tiled.c lpp-spritesheet.c		\
		entity-patch.c

TESTS_CFLAGS += -DTESTS_PATH=\"$(TESTS_DIR)\"

TESTS_OBJ=$(call c_to_o_dir,$(TESTS_DIR),$(TESTS_SRC))

$(TESTS_OBJ): CFLAGS += $(TESTS_CFLAGS) -Wno-pointer-to-int-cast

build-tests: $(TESTS_OBJ) $(YIRL_LINKING)
	$(CC)  -o  $(TESTS_NAME) $(TESTS_OBJ) $(BINARY_LINKING) $(LDFLAGS) -l$(NAME)

clean-tests:
	rm -rvf $(TESTS_OBJ)
