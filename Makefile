NAME= yirl

LIBNAME= lib$(NAME)

all:	build-generic-loader build-tests build-shooter build-sm-mod-ex build-snake-ex

include config.mk

include $(TESTS_DIR)/tests.mk
SHOOTER_DIR=example/shooter/
include $(SHOOTER_DIR)/shooter.mk
SM_MOD_EX_DIR=example/modules/smReader
include $(SM_MOD_EX_DIR)/Makefile
SNAKE_DIR=example/snake
include $(SNAKE_DIR)/Makefile

CFLAGS += -std=gnu11

SRC = 	$(SCRIPT_DIR)/lua-script.c \
	$(SCRIPT_DIR)/entity-script.c \
	$(SCRIPT_DIR)/lua-binding.c \
	$(SCRIPT_DIR)/lua-convert.c \
	$(SCRIPT_DIR)/tcc-script.c \
	$(SCRIPT_DIR)/native-script.c \
	$(SCRIPT_DIR)/ybytecode-script.c \
	$(SCRIPT_DIR)/script.c \
	$(BYTECODE_DIR)/ybytecode.c \
	$(DESCRIPTION_DIR)/description.c \
	$(DESCRIPTION_DIR)/json-desc.c	 \
	$(ENTITY_DIR)/entity.c \
	$(GAME_DIR)/game.c \
	$(UTIL_DIR)/util.c \
	$(UTIL_DIR)/block-array.c \
	$(UTIL_DIR)/debug.c \
	$(WID_DIR)/widget.c \
	$(WID_DIR)/widget-callback.c \
	$(WID_DIR)/text-screen.c \
	$(WID_DIR)/menu.c \
	$(WID_DIR)/map.c \
	$(WID_DIR)/pos.c \
	$(WID_DIR)/rect.c \
	$(WID_DIR)/contener.c \
	$(SDL_WID_DIR)/sdl.c \
	$(SDL_WID_DIR)/map.c \
	$(SDL_WID_DIR)/menu.c \
	$(SDL_WID_DIR)/text-screen.c \
	$(CURSES_DIR)/curses.c \
	$(CURSES_DIR)/text-screen.c \
	$(CURSES_DIR)/menu.c \
	$(CURSES_DIR)/map.c

SRC += $(SOUND_SRC)

OBJ =   $(SRC:.c=.o)

GEN_LOADER_SRC = $(GEN_LOADER_DIR)/main.c
GEN_LOADER_OBJ = $(GEN_LOADER_SRC:.c=.o)

LDFLAGS += $(TCC_LIB_PATH)$(TCC_LIB_NAME)
LDFLAGS += -L./
LDFLAGS += $(shell $(PKG_CONFIG) --libs glib-2.0)
LDFLAGS += $(LUA_LIB)
LDFLAGS += $(VLC_LIB)
LDFLAGS += $(shell $(PKG_CONFIG) --libs json-c)
LDFLAGS += $(CURSES_LIB)
LDFLAGS += $(NUMA_LIB)
LDFLAGS += $(shell $(PKG_CONFIG) --libs SDL2_image)
LDFLAGS += $(shell $(PKG_CONFIG) --libs SDL2_ttf)
LDFLAGS += $(LDFLAGS_EXT)

CFLAGS += $(shell $(PKG_CONFIG) --cflags glib-2.0)
CFLAGS += -I$(YIRL_INCLUDE_PATH)
CFLAGS += -I$(YIRL_INCLUDE_PATH2)
CFLAGS += -I$(TCC_LIB_PATH)
CFLAGS += -fpic

CFLAGS += -DYIRL_INCLUDE_PATH=\"$(YIRL_INCLUDE_PATH2)\"
CFLAGS += -DTCC_LIB_PATH=\"$(TCC_LIB_PATH)\"

build-static-lib: $(OBJ)
	$(AR)  -r -c -s $(LIBNAME).a $(OBJ)

build-dynamic-lib: $(OBJ)
	$(CC) -shared -o  $(LIBNAME).$(LIBEXTENSION) $(OBJ) $(LDFLAGS)

build-generic-loader: $(YIRL_LINKING) $(GEN_LOADER_OBJ)
	$(CC) -o yirl-loader$(BIN_EXT) $(GEN_LOADER_OBJ) -l$(NAME) $(LDFLAGS)

clean:	clean-tests
	rm -rvf $(OBJ)

fclean: clean
	rm -rvf $(LIBNAME).a $(LIBNAME).so $(LIBNAME).dll
