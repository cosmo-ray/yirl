# YIRL isn't a Rogue-like

YIRL is a WIP,
This README is a WIP.
Don't wait to get something functional for now :)

YIRL is a game engine aiming to be fully configruable, fully scriptable and
fully modfiable.

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
