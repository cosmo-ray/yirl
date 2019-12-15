NAME= yirl

LIBNAME= lib$(NAME)

all:	yirl-loader build-tests build-shooter \
	build-sm-mod-ex build-snake-ex

include config.mk

include $(TESTS_DIR)/tests.mk
SHOOTER_DIR=example/shooter/
include $(SHOOTER_DIR)/shooter.mk
SM_MOD_EX_DIR=example/modules/smReader
include $(SM_MOD_EX_DIR)/Makefile
SNAKE_DIR=example/snake
include $(SNAKE_DIR)/Makefile

SRC = 	$(SCRIPT_DIR)/lua-script.c \
	$(SCRIPT_DIR)/entity-script.c \
	$(SCRIPT_DIR)/lua-binding.c \
	$(SCRIPT_DIR)/lua-convert.c \
	$(SCRIPT_DIR)/tcc-script.c \
	$(SCRIPT_DIR)/tcc-syms.c \
	$(SCRIPT_DIR)/native-script.c \
	$(SCRIPT_DIR)/ybytecode-script.c \
	$(SCRIPT_DIR)/s7-script.c \
	$(SCRIPT_DIR)/duk-script.c \
	$(SCRIPT_DIR)/script.c \
	$(BYTECODE_DIR)/ybytecode.c \
	$(BYTECODE_DIR)/condition.c \
	$(DESCRIPTION_DIR)/description.c \
	$(DESCRIPTION_DIR)/json-desc.c	 \
	$(DESCRIPTION_DIR)/rawfile-decs.c \
	$(ENTITY_DIR)/entity.c \
	$(ENTITY_DIR)/entity-string.c \
	$(ENTITY_DIR)/entity-convertions.c \
	$(ENTITY_DIR)/entity-patch.c \
	$(GAME_DIR)/game.c \
	$(UTIL_DIR)/util.c \
	$(UTIL_DIR)/block-array.c \
	$(UTIL_DIR)/debug.c \
	$(WID_DIR)/widget.c \
	$(WID_DIR)/text-screen.c \
	$(WID_DIR)/menu.c \
	$(WID_DIR)/map.c \
	$(WID_DIR)/pos.c \
	$(WID_DIR)/rect.c \
	$(WID_DIR)/events.c \
	$(WID_DIR)/container.c \
	$(WID_DIR)/texture.c \
	$(WID_DIR)/canvas.c \
	$(SDL_WID_DIR)/sdl.c \
	$(SDL_WID_DIR)/map.c \
	$(SDL_WID_DIR)/menu.c \
	$(SDL_WID_DIR)/text-screen.c \
	$(SDL_WID_DIR)/canvas.c

SRC += $(CURSES_SRC)

SRC += $(SOUND_SRC)

O_SRC = $(SCRIPT_DIR)/s7.c

O_OBJ = $(O_SRC:.c=.o)

DUCK_SRC = $(DUCK_V)/src/duktape.c \
	   $(DUCK_V)/extras/print-alert/duk_print_alert.c \
	   $(DUCK_V)/extras/console/duk_console.c

DUK_OBJ =  $(DUCK_SRC:.c=.o)

DUK_FLAGS =  -fPIC -Os -g -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer

SRCXX += 	$(ENTITY_DIR)/entity-cplusplus.cpp

CXX = $(CC)

OBJ =   $(SRC:.c=.o)
OBJXX = $(SRCXX:.cpp=.o)

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
LDFLAGS += $(shell $(PKG_CONFIG) --libs SDL2_mixer)
LDFLAGS += $(LDFLAGS_EXT)
LDFLAGS += $(LIBS_SAN)

COMMON_CFLAGS += $(shell $(PKG_CONFIG) --cflags glib-2.0)
COMMON_CFLAGS += -I$(YIRL_INCLUDE_PATH)
COMMON_CFLAGS += -I$(YIRL_INCLUDE_PATH2)
COMMON_CFLAGS += -I$(TCC_LIB_PATH)
COMMON_CFLAGS += -I./core/script/
COMMON_CFLAGS += -I./$(DUCK_V)/ -I./$(DUCK_V)/src/ # <--- last one is here so I can compile extras
COMMON_CFLAGS += -fpic
COMMON_CFLAGS += $(LUA_CFLAGS)
COMMON_CFLAGS += -Werror -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast

COMMON_CFLAGS += -DYIRL_INCLUDE_PATH=\"$(YIRL_INCLUDE_PATH2)\"
COMMON_CFLAGS += -DTCC_LIB_PATH=\"$(TCC_LIB_PATH)\"
COMMON_CFLAGS += $(FLAGS_SAN)
COMMON_CFLAGS += -Wno-unknown-warning-option
COMMON_CFLAGS += -fno-strict-aliasing # casting entity doesn't really respect strict aliasing rules

CXXFLAGS = $(COMMON_CFLAGS) -x c++ -Wno-missing-exception-spec -fno-exceptions -fno-rtti

CFLAGS += $(COMMON_CFLAGS) -std=gnu11 -D_GNU_SOURCE

DUCK_V = duktape-2.3.0

get-duck:
	wget "https://duktape.org/$(DUCK_V).tar.xz"

$(DUCK_V): get-duck
	tar xvfJ $(DUCK_V).tar.xz

$(DUCK_V)/src/duktape.o:
	$(CC) -c -o $(DUCK_V)/src/duktape.o $(DUCK_V)/src/duktape.c $(DUK_FLAGS)

$(DUCK_V)/extras/print-alert/duk_print_alert.o:
	$(CC) -c -o $(DUCK_V)/extras/print-alert/duk_print_alert.o $(DUCK_V)/extras/print-alert/duk_print_alert.c $(DUK_FLAGS) -I./$(DUCK_V)/src/

$(DUCK_V)/extras/print-alert/duk_console.o:
	$(CC) -c -o $(DUCK_V)/extras/console/duk_console.o $(DUCK_V)/extras/console/duk_console.c $(DUK_FLAGS) -I./$(DUCK_V)/src/

$(SCRIPT_DIR)/s7.o:
	$(CC) -c -o $(SCRIPT_DIR)/s7.o $(SCRIPT_DIR)/s7.c -Wno-implicit-fallthrough -fPIC -O0 -g

build-static-lib: $(OBJ) $(O_OBJ) $(OBJXX) $(DUK_OBJ)
	$(AR)  -r -c -s $(LIBNAME).a $(OBJ) $(O_OBJ) $(OBJXX) $(DUK_OBJ)

build-dynamic-lib: $(OBJ) $(O_OBJ) $(OBJXX) $(DUK_OBJ)
	$(CC) -shared -o  $(LIBNAME).$(LIBEXTENSION) $(OBJ) $(O_OBJ) $(OBJXX) $(DUK_OBJ) $(LDFLAGS)

yirl-loader: $(YIRL_LINKING) $(GEN_LOADER_OBJ)
	$(CC) -o yirl-loader$(BIN_EXT) $(GEN_LOADER_OBJ) $(BINARY_LINKING) $(LDFLAGS)

clean:	clean-tests clean-shooter
	rm -rvf $(OBJ) $(OBJXX) $(GEN_LOADER_OBJ)

fclean: clean
	rm -rvf $(LIBNAME).a $(O_OBJ) $(DUCK_OBJ) $(LIBNAME).so $(LIBNAME).dll
