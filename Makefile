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

SRC = 	$(SCRIPT_DIR)/lua-script.c \
	$(SCRIPT_DIR)/entity-script.c \
	$(SCRIPT_DIR)/lua-binding.c \
	$(SCRIPT_DIR)/lua-convert.c \
	$(SCRIPT_DIR)/tcc-script.c \
	$(SCRIPT_DIR)/tcc-syms.c \
	$(SCRIPT_DIR)/native-script.c \
	$(SCRIPT_DIR)/ybytecode-script.c \
	$(SCRIPT_DIR)/script.c \
	$(BYTECODE_DIR)/ybytecode.c \
	$(BYTECODE_DIR)/condition.c \
	$(DESCRIPTION_DIR)/description.c \
	$(DESCRIPTION_DIR)/json-desc.c	 \
	$(DESCRIPTION_DIR)/rawfile-decs.c \
	$(ENTITY_DIR)/entity.c \
	$(ENTITY_DIR)/entity-string.c \
	$(ENTITY_DIR)/entity-convertions.c \
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
	$(WID_DIR)/container.c \
	$(WID_DIR)/texture.c \
	$(SDL_WID_DIR)/sdl.c \
	$(SDL_WID_DIR)/map.c \
	$(SDL_WID_DIR)/menu.c \
	$(SDL_WID_DIR)/text-screen.c \

SRC += $(CURSES_SRC)

SRC += $(SOUND_SRC)

SRCXX += 	$(ENTITY_DIR)/entity-cplusplus.cpp \
		$(WID_DIR)/canvas.cpp \
		$(SDL_WID_DIR)/canvas.cpp

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
COMMON_CFLAGS += -fpic
COMMON_CFLAGS += $(LUA_CFLAGS)
COMMON_CFLAGS += -Werror -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast

COMMON_CFLAGS += -DYIRL_INCLUDE_PATH=\"$(YIRL_INCLUDE_PATH2)\"
COMMON_CFLAGS += -DTCC_LIB_PATH=\"$(TCC_LIB_PATH)\"
COMMON_CFLAGS += $(FLAGS_SAN)

CXXFLAGS = $(COMMON_CFLAGS) -x c++ -Wno-missing-exception-spec

CFLAGS += $(COMMON_CFLAGS) -std=gnu11

build-static-lib: $(OBJ) $(OBJXX)
	$(AR)  -r -c -s $(LIBNAME).a $(OBJ) $(OBJXX)

build-dynamic-lib: $(OBJ) $(OBJXX)
	$(CC) -shared -o  $(LIBNAME).$(LIBEXTENSION) $(OBJ) $(OBJXX) $(LDFLAGS)

build-generic-loader: $(YIRL_LINKING) $(GEN_LOADER_OBJ)
	$(CC) -o yirl-loader$(BIN_EXT) $(GEN_LOADER_OBJ) $(BINARY_LINKING) $(LDFLAGS)

clean:	clean-tests clean-shooter
	rm -rvf $(OBJ) $(OBJXX) $(GEN_LOADER_OBJ)

fclean: clean
	rm -rvf $(LIBNAME).a $(LIBNAME).so $(LIBNAME).dll
