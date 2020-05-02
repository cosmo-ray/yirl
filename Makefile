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
	$(SCRIPT_DIR)/quickjs.c \
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
	$(UTIL_DIR)/math.c \
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

SRCXX += 	$(ENTITY_DIR)/entity-cplusplus.cpp

CXX = $(CC)

OBJ =   $(SRC:.c=.o)
OBJXX = $(SRCXX:.cpp=.o)

QUICKJS_V = 2020-03-16

QUICKJS_PATH = quickjs-$(QUICKJS_V)
QUICKJS_LIB_PATH = $(QUICKJS_PATH)/libquickjs.a

GEN_LOADER_SRC = $(GEN_LOADER_DIR)/main.c
GEN_LOADER_OBJ = $(GEN_LOADER_SRC:.c=.o)

#../SDL_mixer/build/.libs/libSDL2_mixer-2.0.so.0.2.2
#SDL_MIXER_LDFLAGS = "/home/uso/SDL_mixer/build/.libs/libSDL2_mixer.a"
#SDL_MIXER_CFLAGS = "-I../SDL_mixer/"

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
LDFLAGS += $(SDL_MIXER_LDFLAGS) #  $(shell $(PKG_CONFIG) --libs SDL2_mixer)
LDFLAGS += $(LDFLAGS_EXT)
LDFLAGS += $(LIBS_SAN) -ldl $(QUICKJS_LIB_PATH)

COMMON_CFLAGS += $(shell $(PKG_CONFIG) --cflags glib-2.0)
COMMON_CFLAGS += -I$(YIRL_INCLUDE_PATH)
COMMON_CFLAGS += -I$(YIRL_INCLUDE_PATH2)
COMMON_CFLAGS += -I$(TCC_LIB_PATH)
COMMON_CFLAGS += -I./core/script/
COMMON_CFLAGS += -I./$(DUCK_V)/ -I./$(DUCK_V)/src/ # <--- last one is here so I can compile extras
COMMON_CFLAGS += -I./$(QUICKJS_PATH)
COMMON_CFLAGS += -fpic
COMMON_CFLAGS += $(LUA_CFLAGS)
COMMON_CFLAGS += -Werror -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast

COMMON_CFLAGS += -DYIRL_INCLUDE_PATH=\"$(YIRL_INCLUDE_PATH2)\"
COMMON_CFLAGS += -DTCC_LIB_PATH=\"$(TCC_LIB_PATH)\"
COMMON_CFLAGS += $(FLAGS_SAN)
COMMON_CFLAGS += -Wno-unknown-warning-option
COMMON_CFLAGS += -Wno-cast-function-type
COMMON_CFLAGS += -fno-strict-aliasing # casting entity doesn't really respect strict aliasing rules

CXXFLAGS = $(COMMON_CFLAGS) -x c++ -Wno-missing-exception-spec -fno-exceptions -fno-rtti

CFLAGS += $(COMMON_CFLAGS) -std=gnu11 -D_GNU_SOURCE

$(QUICKJS_PATH):
	git clone https://github.com/cosmo-ray/quickjs.git quickjs-$(QUICKJS_V)

$(QUICKJS_LIB_PATH): $(QUICKJS_PATH)
	CONFIG_FPIC=1 make -C $(QUICKJS_PATH) libquickjs.a

$(SCRIPT_DIR)/s7.o:
	$(CC) -c -o $(SCRIPT_DIR)/s7.o $(SCRIPT_DIR)/s7.c -Wno-implicit-fallthrough -fPIC -O0 -g

build-static-lib: $(OBJ) $(O_OBJ) $(OBJXX) $(QUICKJS_LIB_PATH)
	$(AR)  -r -c -s $(LIBNAME).a $(OBJ) $(O_OBJ) $(OBJXX) $(QUICKJS_LIB_PATH)

build-dynamic-lib: $(OBJ) $(O_OBJ) $(OBJXX) $(QUICKJS_LIB_PATH)
	$(CC) -shared -o  $(LIBNAME).$(LIBEXTENSION) $(OBJ) $(O_OBJ) $(OBJXX) $(LDFLAGS)

yirl-loader: $(YIRL_LINKING) $(GEN_LOADER_OBJ)
	$(CC) -o yirl-loader$(BIN_EXT) $(GEN_LOADER_OBJ) $(BINARY_LINKING) $(LDFLAGS)

clean:	clean-tests clean-shooter
	rm -rvf $(OBJ) $(OBJXX) $(GEN_LOADER_OBJ)

fclean: clean
	rm -rvf $(LIBNAME).a $(O_OBJ) $(LIBNAME).so $(LIBNAME).dll

clean_all: fclean
	rm -rvf $(DUCK_OBJ) $(QUICKJS_LIB_PATH)
