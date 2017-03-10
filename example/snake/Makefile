SNAKE_CFLAGS+= -DTESTS_PATH=\"$(SNAKE_DIR)\"

SNAKE_SRC= main.c

SNAKE_OBJ=$(call c_to_o_dir,$(SNAKE_DIR),$(SNAKE_SRC))

build-snake-ex: $(SNAKE_OBJ)
	$(CC)  -o  snake-ex $(SNAKE_OBJ) $(LDFLAGS) -l$(NAME)
