NAME= yirl

LIBNAME= lib$(NAME)

all:	yirl-loader build-tests

include config.mk

include $(TESTS_DIR)/tests.mk

SRC = 	$(HSEARCH_SRC) \
	$(SCRIPT_DIR)/lua-script.c \
	$(SCRIPT_DIR)/entity-script.c \
	$(SCRIPT_DIR)/lua-binding.c \
	$(SCRIPT_DIR)/lua-convert.c \
	$(SCRIPT_DIR)/tcc-script.c \
	$(SCRIPT_DIR)/tcc-syms.c \
	$(SCRIPT_DIR)/native-script.c \
	$(SCRIPT_DIR)/ybytecode-script.c \
	$(SCRIPT_DIR)/s7-script.c \
	$(SCRIPT_DIR)/ph7-script.c \
	$(SCRIPT_DIR)/quickjs.c \
	$(SCRIPT_DIR)/script.c \
	$(PERL_SRC) \
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
	$(UTIL_DIR)/simple-net.c \
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

SRC += $(SOUND_SRC)

O_SRC = $(S7_SOURCE) $(PH7_SOURCE)

O_OBJ = $(O_SRC:.c=.o)

OBJ =   $(SRC:.c=.o)

QUICKJS_V = 2020-03-16

QUICKJS_PATH = quickjs-$(QUICKJS_V)
QUICKJS_LIB_PATH = $(QUICKJS_PATH)/libquickjs.a

GEN_LOADER_SRC = $(GEN_LOADER_DIR)/main.c
GEN_LOADER_OBJ = $(GEN_LOADER_SRC:.c=.o)

#../SDL_mixer/build/.libs/libSDL2_mixer-2.0.so.0.2.2
#SDL_MIXER_LDFLAGS = "/home/uso/SDL_mixer/build/.libs/libSDL2_mixer.a"
#SDL_MIXER_CFLAGS = "-I../SDL_mixer/"

LDFLAGS += $(TCC_LIB_PATH)$(TCC_LIB_NAME)
LDFLAGS += $(shell $(PKG_CONFIG) --libs SDL2_image SDL2_ttf $(WIN_SDL_EXTRA)) $(WIN_SDL_EXTRA2)
LDFLAGS += $(SDL_MIXER_LDFLAGS) $(SDL_MIXER_ARFLAGS)
LDFLAGS += $(SDL_GPU_LDFLAGS)
LDFLAGS += -L./
LDFLAGS += $(LUA_LIB)
LDFLAGS += $(JSON_C_LD)
LDFLAGS += $(LDFLAGS_EXT)
LDFLAGS += $(LIBS_SAN) -ldl $(QUICKJS_LIB_PATH)
LDFLAGS += $(ANALYZER_FLAG)
LDFLAGS += $(EMPORT)
LDFLAGS += $(shell $(PKG_CONFIG) --libs gl glu)
LDFLAGS += $(PERL_LD)
GLIB_LDFLAGS += $(shell $(PKG_CONFIG) --libs glib-2.0)

COMMON_CFLAGS += $(SDL_MIXER_CFLAGS) $(HSEARCH_CFLAGS)
GLIB_COMMON_CFLAGS += $(shell $(PKG_CONFIG) --cflags glib-2.0)
COMMON_CFLAGS += $(shell sdl2-config --cflags)
COMMON_CFLAGS += -I$(YIRL_INCLUDE_PATH)
COMMON_CFLAGS += -I$(YIRL_INCLUDE_PATH2)
COMMON_CFLAGS += $(TCC_CFLAGS) $(S7_CFLAGS) $(PERL_CFLAGS)
COMMON_CFLAGS += -I./core/script/
COMMON_CFLAGS += -I./$(DUCK_V)/ -I./$(DUCK_V)/src/ # <--- last one is here so I can compile extras
COMMON_CFLAGS += $(JSON_C_CFLAGS)
COMMON_CFLAGS += -I./$(QUICKJS_PATH)
COMMON_CFLAGS += -fpic
COMMON_CFLAGS += $(LUA_CFLAGS)
COMMON_CFLAGS += -I$(SDL_GPU_CFLAGS)
COMMON_CFLAGS += $(WERROR) -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast

COMMON_CFLAGS += -DYIRL_INCLUDE_PATH=\"$(YIRL_INCLUDE_PATH2)\"
COMMON_CFLAGS += -DTCC_LIB_PATH=\"$(TCC_LIB_PATH)\"
COMMON_CFLAGS += $(FLAGS_SAN)
COMMON_CFLAGS += -Wno-unknown-warning-option
COMMON_CFLAGS += -Wno-cast-function-type
COMMON_CFLAGS += -fno-strict-aliasing # casting entity doesn't really respect strict aliasing rules
COMMON_CFLAGS += $(ANALYZER_FLAG)
COMMON_CFLAGS += -I./ph7/
COMMON_CFLAGS += $(EMPORT)

CFLAGS += $(COMMON_CFLAGS) -std=gnu11 -D_GNU_SOURCE $(EMCFLAGS)

INSTALL_MOD=$(PREFIX)/share/yirl/modules/
SCRIPT_DEP=$(PREFIX)/share/yirl/scripts-dependancies/
#this one is here so my screen don't cur the install line
ULPCS=Universal-LPC-spritesheet/

libc_hsearch_r/search_hsearch_r.c:
	git clone https://github.com/mikhail-j/libc_hsearch_r.git

json-c-git/:
	git clone $(JSON_C_GIT) json-c-git

json-c-build/: json-c-git/
	$(EMCMAKE) cmake -B json-c-build json-c-git/

json-c-build/libjson-c.a: json-c-build/
	cd json-c-build && $(EMMAKE) make

lua-git/:
	git clone $(LUA_GIT) lua-git

lua-git/liblua.a:
	cd lua-git && pwd && git fetch origin
	cd lua-git && pwd && git checkout -f v5.4.0
	cd lua-git && cat makefile | sed 's/CC= gcc/#CC variable remove/' | sed 's/-DLUA_USE_READLINE//' | sed 's/-lreadline//' | sed 's/-O2/-O2 -fPIC/g'  > makefile_tmp
	mv lua-git/makefile_tmp lua-git/makefile
	cd lua-git && $(EMMAKE) make

sdl-gpu-build:
	bash -c "$(EMCMAKE) cmake -B ./sdl-gpu-build ./sdl-gpu/ $(CMAKE_ARGS)"

$(SDL_GPU_LDFLAGS): sdl-gpu-build
	$(EMMAKE) make -C sdl-gpu-build 

$(QUICKJS_PATH):
	git clone https://github.com/cosmo-ray/quickjs.git quickjs-$(QUICKJS_V)

$(QUICKJS_LIB_PATH): $(QUICKJS_PATH)
	CONFIG_FPIC=1 $(EMMAKE) make -C $(QUICKJS_PATH) libquickjs.a

ph7/ph7.o:
	$(CC) -c -o ph7/ph7.o ph7/ph7.c -I./ph7/ -O2 -g -fPIC -DPH7_ENABLE_MATH_FUNC=1 -DPH7_ENABLE_THREADS=1

$(SCRIPT_DIR)/s7.o:
	$(CC) -c -o $(SCRIPT_DIR)/s7.o $(SCRIPT_DIR)/s7.c -Wno-implicit-fallthrough -fPIC -O2 -g

$(SCRIPT_DIR)/perl.o: $(PERL_SRC)
	$(CC) -c -o $(SCRIPT_DIR)/perl.o $(SCRIPT_DIR)/perl.c $(shell perl -MExtUtils::Embed -e ccopts) $(PERL_CFLAGS) -I$(YIRL_INCLUDE_PATH2) -fPIC

SDL_mixer/:
	git submodule update --init

$(SDL_MIXER_DEP): SDL_mixer/
	cd SDL_mixer/ && $(EMCONFIGURE) ./configure $(SDL_MIXER_CFG) CFLAGS="$(SDL_MIXER_BUILD_CFLAGS)"
	cd SDL_mixer/ && $(EMMAKE) make

clean_sdl_mixer:
	make -C SDL_mixer/ clean

$(OBJ): $(LUA_RULE) $(JSON_C_RULE) $(QUICKJS_LIB_PATH) $(SDL_MIXER_DEP) $(SDL_GPU_LDFLAGS)

$(LIBNAME).a: $(OBJ) $(O_OBJ) $(SDL_MIXER_DEP)
	$(AR)  -r -c -s $(LIBNAME).a $(OBJ) $(O_OBJ)
$(LIBNAME).$(LIBEXTENSION): $(OBJ) $(O_OBJ) $(OBJXX) $(SDL_MIXER_DEP)
	$(CC) -shared -o  $(LIBNAME).$(LIBEXTENSION) $(OBJ) $(O_OBJ) $(OBJXX) $(LDFLAGS)

yirl-loader: $(YIRL_LINKING) $(GEN_LOADER_OBJ)
	$(CC) -o yirl-loader$(BIN_EXT) $(GEN_LOADER_OBJ) $(BINARY_LINKING) $(LDFLAGS)

WEB_MOD_DST ?= "./low_enforcement_agents/"

PRELOAD_EMCC_FILES ?= \
	--preload-file scripts-dependancies/object-wrapper.lua \
	--preload-file modules/smart_cobject/ \
	--preload-file $(WEB_MOD_DST)

WEB_CFLAG = \
	--preload-file ./sazanami-mincho.ttf \
	--use-preload-plugins


WEB_ARG=arguments: ["-d", "$(WEB_MOD_DST)", "-P", "/", "-W", "800", "-H", "600"],
#	arguments: ['-d', './games/asteroide-shooter/', '-P', "/home/uso/yirl/"],

webstart.html: $(YIRL_LINKING) $(GEN_LOADER_OBJ) $(LIBNAME).a
	$(CC) -o webstart.html $(GEN_LOADER_OBJ) $(LIBNAME).a $(LDFLAGS) -lopenal $(PRELOAD_EMCC_FILES) $(WEB_CFLAG) -sDYNAMIC_EXECUTION=2 -sMAIN_MODULE=1 -sTOTAL_MEMORY=1024MB --closure 1

start.html: webstart.html
	cat webstart.html | sed 's|var Module = {|var Module = {\n\t$(WEB_ARG)|g' > start.html

clean:	clean-tests
	rm -rvf $(OBJ) $(OBJXX) $(GEN_LOADER_OBJ)

fclean: clean
	rm -rvf $(LIBNAME).a $(O_OBJ) $(LIBNAME).so $(LIBNAME).dll webstart.* start.html

clean_all: fclean clean_sdl_mixer
	rm -rvf $(DUCK_OBJ) $(QUICKJS_LIB_PATH) sdl-gpu-build $(QUICKJS_PATH)/.obj lua-git/liblua.a

install: yirl-loader
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)$(INSTALL_MOD)
	mkdir -p $(PREFIX)/bin
	cp yirl-loader $(PREFIX)/bin/
	cp libyirl.so $(PREFIX)/lib/
	mkdir -p $(INSTALL_MOD)/8086-emu/
	cp  modules/8086-emu/asm-inst.h $(INSTALL_MOD)/8086-emu/
	cp  modules/8086-emu/asm-tok.h $(INSTALL_MOD)/8086-emu/
	cp  modules/8086-emu/asm.c $(INSTALL_MOD)/8086-emu/
	cp  modules/8086-emu/start.c $(INSTALL_MOD)/8086-emu/
	cp  modules/8086-emu/charset.h $(INSTALL_MOD)/8086-emu/
	mkdir -p $(INSTALL_MOD)/dialogue/
	cp  modules/dialogue/init.c $(INSTALL_MOD)/dialogue/
	cp  modules/dialogue/start.json $(INSTALL_MOD)/dialogue/
	mkdir -p $(INSTALL_MOD)/dialogue-box/
	cp  modules/dialogue-box/arrow_sheet.png $(INSTALL_MOD)/dialogue-box/
	cp  modules/dialogue-box/init.lua $(INSTALL_MOD)/dialogue-box/
	cp  modules/dialogue-box/start.json $(INSTALL_MOD)/dialogue-box/
	mkdir -p $(SCRIPT_DEP)
	cp  scripts-dependancies/object-wrapper.lua $(SCRIPT_DEP)
	cp -rvf include $(PREFIX)/share/yirl/
	mkdir -p $(PREFIX)/share/yirl/tinycc/
	cp tinycc/libtcc1.a $(PREFIX)/share/yirl/tinycc/
	cp DejaVuSansMono.ttf $(PREFIX)/share/yirl/
	cp sazanami-mincho.ttf $(PREFIX)/share/yirl/
	install -D ./yirl-completion.bash $(PREFIX)/share/bash-completion/completions/yirl-loader
	echo "Install everything in: "$(PREFIX)

install_extra_modules:
	mkdir -p $(INSTALL_MOD)
	mkdir -p $(INSTALL_MOD)/c_app
	cp  modules/c_app/* $(INSTALL_MOD)/c_app/
	mkdir -p $(INSTALL_MOD)/snake
	cp  modules/snake/snake.lua $(INSTALL_MOD)/snake/snake.lua
	cp  modules/snake/start.json $(INSTALL_MOD)/snake/start.json
	cp  modules/snake/bg.png $(INSTALL_MOD)/snake/bg.png
	mkdir -p $(INSTALL_MOD)/hightscore/
	cp modules/hightscore/score.lua $(INSTALL_MOD)/hightscore/
	cp modules/hightscore/start.json $(INSTALL_MOD)/hightscore/
	mkdir -p $(INSTALL_MOD)/jrpg-fight/
	cp modules/jrpg-fight/init.lua $(INSTALL_MOD)/jrpg-fight/
	cp modules/jrpg-fight/animation.lua $(INSTALL_MOD)/jrpg-fight/
	cp modules/jrpg-fight/start.json $(INSTALL_MOD)/jrpg-fight/
	cp modules/jrpg-fight/image0007.png $(INSTALL_MOD)/jrpg-fight/
	cp modules/jrpg-fight/image0009.png $(INSTALL_MOD)/jrpg-fight/
	cp modules/jrpg-fight/explosion.png $(INSTALL_MOD)/jrpg-fight/
	cp modules/jrpg-fight/BG_City.jpg $(INSTALL_MOD)/jrpg-fight/
	cp modules/jrpg-fight/README.md $(INSTALL_MOD)/jrpg-fight/
	mkdir -p $(INSTALL_MOD)/sprite-manager/
	cp modules/sprite-manager/start.c $(INSTALL_MOD)/sprite-manager/
	mkdir -p $(INSTALL_MOD)/loading_bar/
	cp modules/loading_bar/start.c $(INSTALL_MOD)/loading_bar/
	mkdir -p $(INSTALL_MOD)/$(ULPCS)/
	cp modules/$(ULPCS)/start.json $(INSTALL_MOD)/$(ULPCS)/
	cp modules/$(ULPCS)/lpcs.lua $(INSTALL_MOD)/$(ULPCS)/
	cp -rvf modules/$(ULPCS)/spritesheets/ $(INSTALL_MOD)/$(ULPCS)/
	cp modules/$(ULPCS)/CREDITS.TXT $(INSTALL_MOD)/$(ULPCS)/
	cp modules/$(ULPCS)/cc-by-sa-3_0.txt $(INSTALL_MOD)/$(ULPCS)/
	mkdir -p $(INSTALL_MOD)/pong/
	cp modules/pong/start.scm $(INSTALL_MOD)/pong/
	mkdir -p $(INSTALL_MOD)/tiled/
	cp modules/tiled/tiled.c $(INSTALL_MOD)/tiled/
	cp modules/tiled/start.json $(INSTALL_MOD)/tiled/
	mkdir -p $(INSTALL_MOD)/vapp/
	cp 'modules/vapp/New Piskel.png' $(INSTALL_MOD)/vapp/
	cp modules/vapp/viking.png $(INSTALL_MOD)/vapp/
	cp modules/vapp/start.json $(INSTALL_MOD)/vapp/
	cp modules/vapp/pizza.png $(INSTALL_MOD)/vapp/
	cp modules/vapp/init.c $(INSTALL_MOD)/vapp/
	cp modules/vapp/resources.json $(INSTALL_MOD)/vapp/
	mkdir -p $(INSTALL_MOD)/shooter/
	cp modules/shooter/DurrrSpaceShip.png $(INSTALL_MOD)/shooter/
	cp modules/shooter/start.lua $(INSTALL_MOD)/shooter/
	cp -rvf modules/shooter/jswars_gfx/ $(INSTALL_MOD)/shooter/
	mkdir -p $(INSTALL_MOD)/asteroide-shooter/
	cp modules/asteroide-shooter/start.json $(INSTALL_MOD)/asteroide-shooter/

.PHONY : install clean_all fclean clean all install_extra_modules build-tests clean-tests clean_sdl_mixer
