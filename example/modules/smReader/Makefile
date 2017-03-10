SM_MOD_EX_CFLAGS+= -DTESTS_PATH=\"$(SM_MOD_EX_DIR)\"

SM_MOD_EX_SRC= main.c

SM_MOD_EX_OBJ=$(call c_to_o_dir,$(SM_MOD_EX_DIR),$(SM_MOD_EX_SRC))

build-sm-mod-ex: $(SM_MOD_EX_OBJ)
	$(CC)  -o  sm-reader-ex $(SM_MOD_EX_OBJ) $(LDFLAGS) -l$(NAME)
