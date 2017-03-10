SHOOTER_NAME = test-shooter

SHOOTER_SRC = main.c shooter.c

SHOOTER_CFLAGS += -DTESTS_PATH=\"$(SHOOTER_DIR)\"

SHOOTER_OBJ=$(call c_to_o_dir,$(SHOOTER_DIR),$(SHOOTER_SRC))

$(SHOOTER_OBJ): CFLAGS += $(SHOOTER_CFLAGS)

build-shooter: $(SHOOTER_OBJ) build-dynamic-lib
	$(CC)  -o  $(SHOOTER_NAME) $(SHOOTER_OBJ) $(LDFLAGS) -l$(NAME)

clean-shooter:
	rm -rvf $(SHOOTER_OBJ)

