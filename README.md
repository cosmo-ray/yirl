# YIRL isn't a Rogue-like

YIRL is a WIP,
This README is a WIP.
Don't wait to get something functional for now :)

YIRL is a game engine aiming to be fully configruable, fully scriptable and
fully modfiable.

Concept:
The idea behind yirl is to give to the user some basic tool and widget that everyone can wrape into more complex tool.
An example can be the snake module which is a basic map, whith more callback and function that allow everyone to add a snake easily in his video game.
We can see yirl as a CMS for video game:
everyone should be able to add module to yirl, and everyone should be able to wrape modules into more powerfull modules.

Licence:
YIRL is licensed under the LGPL licence, the idea behind that, is that you can add a modules under the licence you want, but if you make a modification to the game engine, you need to share it :)

Tree:

* include: headers
  * widget: widgets API
    * widget.h: widget base class and common widgets functions
    * map.h: a square map
    * menu.h
    * text-screen.h: a simple text widget.
    * contener.h: widget that contain others widgets
    * pos.h: function to help to manipulate everything with a pos
    * sdl-driver.h
    * curses-driver.h
    * widget-callback.h
    * keydef.h: keybord touches defines base on curses keynum
  * core: everything exept widgets
    * game.h: the main header, that initialise everything handle yirl's modules
    * entity.h: the yirl entity system is here, use by everything in the engine
    * debug.h: some debug functions
    * description.h: generic api for descriptions files(files that describt entity tree)
    * script.h: generic api for scripting manipulation
    * json-desc.h: json description
    * lua-script.h
    * tcc-script.h
    * lua-binding.h: binding between yirl api and lua
    * block-array.h: yirl internal array use by ArrayEntity
    * sound.h: yirl abstract sound system
    * sound-libvlc.h: sound vlc implementation
    * util.h
    * timer.h
* core: engine sources
* test: unit tests
* modules: set of yirl modules
* example: set of examples
* cmake: cmake dependencies
