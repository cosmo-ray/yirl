//           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//                   Version 2, December 2004
//
// Copyright (C) 2023 Matthias Gatto <uso.cosmo.ray@gmail.com>
//
// Everyone is permitted to copy and distribute verbatim or modified
// copies of this license document, and changing it is allowed as long
// as the name is changed.
//
//            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
//
//  0. You just DO WHAT THE FUCK YOU WANT TO.

let LAST = 5

let UL_ARROW = `^
  \\
   \\
`
let DL_ARROW = `  /
 /
V
`
let UR_ARROW = `  ^
 /
/
`
let DR_ARROW = `\\
 \\
  V
`

let HELP_GUY = `
 o        o
  \\      /
    -^^-
  < Y  Y >
     WW
    \\__/
`

function y_stop_action(wid, eves)
{
    if (yevAnyMouseDown(eves) ||
	yevIsKeyDown(eves, Y_ENTER_KEY) ||
	yevIsKeyDown(eves, Y_SPACE_KEY) ||
	yevIsKeyDown(eves, Y_ESC_KEY)) {
	let data = yeGet(wid, "_stop_data")

	for (i = 0; i < LAST; ++i)
	    ywCanvasRemoveObj(wid, yeGet(data, i))

	yeRemoveChildByStr(wid, "_stop_data");
	yeRemoveChildByStr(wid, "action");
	yeRenameStrStr(wid, "_old_action", "action")
    }
}

function y_stop_func(wid, x, y, txt, have_arrow)
{
    var wid_pos = yeGet(wid, "cam")
    var start_x = 0
    var start_y = 0

    if (wid_pos) {
	start_x = ywPosX(wid_pos)
	start_y = ywPosY(wid_pos)
	wid_pos = yeGet(wid, "cam-threshold")
	if (wid_pos) {
	    start_x = start_x + ywPosX(wid_pos)
	    start_y = start_y + ywPosY(wid_pos)
	}
    }
    print("start_x", start_x)
    print("start_y", start_y)
    wid_pix = yeGet(wid, "wid-pix");
    if (!wid_pix) {
	print("y_stop_helper need to be called after been created !");
	return false
    }

    ww = ywRectW(wid_pix);
    wh = ywRectH(wid_pix);

    yeRenameStrStr(wid, "action", "_old_action");
    yeCreateFunction("y_stop_action", wid, "action");
    let data = yeCreateArray(wid, "_stop_data");
    txt = yeCreateString(txt)
    yePushBack(data, ywCanvasNewRectangle(wid, start_x, start_y, ywRectW(wid_pix),
					  ywRectH(wid_pix),
					  "rgba: 40 100 230 150"));
    var xdir = 1
    var ydir = 1

    if (have_arrow) {
	if (x < ww / 2) { // left
	    if (y <  wh / 2) { // up
		yePushBack(data, ywCanvasNewText(wid, x, y, yeCreateString(UL_ARROW)));
	    } else {
		yePushBack(data, ywCanvasNewText(wid, x, y, yeCreateString(DL_ARROW)));
		ydir = -1
	    }
	} else { // right
	    xdir = -1
	    if (y <  wh / 2) { // up
		yePushBack(data, ywCanvasNewText(wid, x, y, yeCreateString(UR_ARROW)));
	    } else {
		yePushBack(data, ywCanvasNewText(wid, x, y, yeCreateString(DR_ARROW)));
		ydir = -1
	    }
	}
    }

    let txt_js = yeGetString(txt)
    let w = txt_js.indexOf("\n") * 9
    if (w < 0)
	w = txt_js.length * 9;

    yePushBack(data, ywCanvasNewRectangle(wid, x + 18 * xdir, y + 58  * ydir, w,
					  20 * (1 + yeCountLines(txt)),
					  "rgba: 255 255 255 150"));
    yePushBack(data, ywCanvasNewText(wid, x + 20  * xdir,
				     y + 60 * ydir, txt));
    var head_threshold = 100
    if (ydir < 0) {
	head_threshold = -w
    } else {
	head_threshold = 20 * (1 + yeCountLines(txt)) + 40
    }
    yePushBack(data, ywCanvasNewText(wid, x + 70  * xdir,
				     y + head_threshold, yeCreateString(HELP_GUY)));
    return true
}

function y_stop_helper(wid, x, y, txt)
{
    return y_stop_func(wid, x, y, txt, true)
}

function y_stop_head(wid, x, y, txt)
{
    return y_stop_func(wid, x, y, txt, false)
}

function mod_init(mod)
{
    ygRegistreFunc(4, "y_stop_head", "y_stop_head");
    ygRegistreFunc(4, "y_stop_helper", "y_stop_helper");
    return mod
}
