# YIRL isn't a Rogue-like

YIRL is a WIP,

YIRL is a game engine aiming to be fully configurable, fully scriptable and mod friendly.

# Concept:
The idea behind yirl is to give to the user some basic tools and widgets that everyone can reuse or wrap into more complex widgets.
We can see yirl as a CMS for video game:
Everyone should be able to add module to yirl, and everyone should be able to wrap modules into more powerful modules.

As an example the [snake module](https://github.com/cosmo-ray/yirl/tree/master/modules/snake) is a map whith more function and a init, it's basically an inheritance of a map, this allow everyone to add a snake easily in his game.
Snake module is written in lua, but as YIRL have a generic script system, we could have written snake in any scripting language.
Here is an example how sanke module can be use: https://github.com/cosmo-ray/yirl/tree/master/example/modules/snake.

Now only lua and C with tcc are supported for scripting, feel free to add your language :).


# Licence:
YIRL is licensed under the LGPL licence, the idea behind that, is that you can add a modules under the licence you want, but if you make a modification to the engine, you need to share it.

# Tree:

* include: headers
  * widget: widgets API
    * widget.h: widget base class and common widgets functions
    * map.h: a square map
    * menu.h
    * text-screen.h: a simple text widget.
    * container.h: widget that contain others widgets
    * pos.h: helpers to manipulate everything with a position
    * rect.h: helpers for rectangles
    * sdl-driver.h
    * curses-driver.h
    * widget-callback.h
    * keydef.h: keyboard touches define base on curses key numbers
  * core: everything exept widgets
    * game.h: the main header, initialise everything and manage yirl's modules
    * entity.h: the yirl entity system, use by everything in the engine
    * debug.h: some debug functions
    * description.h: generic api for descriptions files(files that describt entitys tree)
    * json-desc.h
    * script.h: generic api for scripting manipulation
    * lua-script.h
    * tcc-script.h
    * native-script.h: allow to create FunctionEntity from YIRL internal API
    * lua-binding.h: binding between yirl api and lua
    * block-array.h: yirl internal array use by ArrayEntity
    * sound.h: yirl abstract sound system
    * sound-libvlc.h: libvlc sound implementation
    * util.h
    * timer.h
* core: engine sources
* test: unit tests
* modules: set of basic yirl modules
* games: Might be playable someday
  * sukeban: aiming to create a J-RPG
  * LoGH: for Legend of the Galatic Battle will be a great HOMM-like game with space fleet
  * vapz: vikings again pinapple pizza: https://uso.itch.io/icelandic-viking-vs-pineapple-pizza
* example:
  * shooter: old oudated example usefull for testing purpose
  * snake: example that show how to modifie a map in order to implement a snake, module/snake is a modified version of this file
  * modules: examples of yirl modules usage
    * snake: how to use the snake module
    * sm-reader: how to use sm-reader

# Contribution

Obviouselly contributions are more than welcome, to contribute you can simply make a Pull Request on github, if something is wrong, I'll tell you :)
As I didn't have time to write a coding style, in case of contribution, you should just try to imitate code alerady in place.

I'd like to use linux conding style, but this would require to change every functions and structures names.
A good contribution would be to make yirl compatible with linux coding style.

# Projects using YIRL
* https://uso.itch.io/pre-hangover-simulator

# Dependancies

Devlopement package of these libs:

* glib2
* lua 53+
* sdl, sdl_image, sdl_ttf, sdl_mixer

Optinal:
* ncurses

# building
Linux - Mac Os:
```
git submodule update --init
./configure
make
```

Due to the way Ubuntu package lua, you need to do `./configure -t ubuntu` instead of `./configure`
If you don't have ncurses, use `./configure WITH_CURSES=0`

Windows:
you need to use msys2: http://www.msys2.org/
```
git submodule update --init
./configure -t mingw-i686
make
```

# chat with us
* irc: #yirl on freenode
* discord: https://discord.gg/8QrKTtV


# how to start using yirl
For now the easier way to use yirl is to come chat with us and ask for help
But you can still understand how yirl work by:
* using examples
* reading examples json file, and scripting files (either lua, C or yb)
* copy past an example and change it to create your own game
* patch evey things that dont work

# Ideas of Contributions/TODO

* use linux coding style
* create a module that allow to use web radio
* add isometric view in map
* add canvas widget, with an api base on html5 canvas
* add an hex map
* add any other cool widget
* use vulkan/gl
* fix all broken stuff in curses
* add lisp or any other languge scripting support
* add a tetris module or any other cool game template module

