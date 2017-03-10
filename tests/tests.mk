TESTS_NAME = yirl-tests

TESTS_SRC =  main.c lifecycle.c stringOperations.c test-copy.c setunset.c	\
		lua-script.c tcc-script.c					\
		test-json-desc.c test-widgets.c menu.c map.c			\
		test-game.c block-array.c contener.c test-sound.c		\
		ybytecode.c script.c

TESTS_CFLAGS += -DTESTS_PATH=\"$(TESTS_DIR)\"

TESTS_OBJ=$(call c_to_o_dir,$(TESTS_DIR),$(TESTS_SRC))

$(TESTS_OBJ): CFLAGS += $(TESTS_CFLAGS)

build-tests: $(TESTS_OBJ) build-dynamic-lib
	$(CC)  -o  $(TESTS_NAME) $(TESTS_OBJ) $(LDFLAGS) -l$(NAME)

clean-tests:
	rm -rvf $(TESTS_OBJ)
