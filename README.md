# YIRL isn't a Rogue-like

YIRL is a WIP,
This README is a WIP.
Don't expect to get something functional for now.

YIRL is a game engine aiming to be fully configurable, fully scriptable and mod friendly.

# Concept:
The idea behind yirl is to give to the user some basic tools and widgets that everyone can reuse or wrap into more complex widgets.
We can see yirl as a CMS for video game:
Everyone should be able to add module to yirl, and everyone should be able to wrap modules into more powerful modules.

As an example the [snake module](https://github.com/cosmo-ray/yirl/tree/master/modules/snake) is a map whith more function and a init, it's basically an inheritance of a map, this allow everyone to add a snake easily in his game.
Snake module is written in lua, but as YIRL have a generic script system, we could have written snake in any scripting language.
Here is an example how sanke module can be use: https://github.com/cosmo-ray/yirl/tree/master/example/modules/snake
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
    * contener.h: widget that contain others widgets
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
* modules: set of yirl modules
* example: set of examples
* cmake: cmake dependencies
