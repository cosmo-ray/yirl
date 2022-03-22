# YIRL dialogue modules.

## brief

This modules parse a dialogue tree, and render it either in a canvas (using canvas-box module) or in text-screen + menu container.

to use it in canvas mode create a `dialogue-canvas` widget, in a container a `dialogue` widget.

## examples

you can look at https://github.com/cosmo-ray/yirl/blob/master/modules/dialogue/blabla.json
or dialogues in sukeban for more complex dialogues: https://github.com/cosmo-ray/Sukeban/tree/d1c680f72e3f0eba936fa52b63aad9856a6e1d6f/dialogue

*Note that sukeban dialoges are not widgets description, but just the dialogue fiel a of widget dialogues*

## callback

dialogue heavilly rely on callback, you can either use:

- YIRL callback (`setInt`, `recreateInt`, `recreateString` ...)
- dialogue module callback (`dialogue.change-text`, `dialogue.goto`, `dialogue.gotoNext`...)
- create you own

## create you own callback

When creating a dialogue callback, the callback will have the widget/box as first argument, and the current answer as second.
