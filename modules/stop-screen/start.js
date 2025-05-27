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

const LAST = 5

const UL_ARROW = `^
  \\
   \\
`
const DL_ARROW = `  /
 /
V
`
const UR_ARROW = `  ^
 /
/
`
const DR_ARROW = `\\
 \\
  V
`

const HELP_GUY = `
 o        o
  \\      /
    -^^-
  < Y  Y >
     WW
    \\__/
`

let CUR_HEAD = HELP_GUY
let HEAD_DEFAULT_COLOR = "rgba: 255 255 255 150"
let HEAD_RECT_COLOR = HEAD_DEFAULT_COLOR
let HEAD_RECT_FLAG = 1

function y_set_head(head)
{
    CUR_HEAD = head
}

function y_set_talk_rect_style(color, flag)
{
    HEAD_RECT_COLOR = color
    HEAD_RECT_FLAG = flag
}

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
    let wid_pos = yeGet(wid, "cam")
    let start_x = 0
    let start_y = 0

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
    yePushBack(data, ywCanvasNewRectangleExt(wid, start_x, start_y, ywRectW(wid_pix),
					     ywRectH(wid_pix),
					     "rgba: 40 100 230 150",
					     1));
    let xdir = 1
    let ydir = 1

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

    yePushBack(data, ywCanvasNewRectangleExt(wid, x + 10 * xdir, y + 50  * ydir, w + 5,
					     20 * (1 + yeCountLines(txt)) + 10,
					     HEAD_RECT_COLOR, HEAD_RECT_FLAG));
    yePushBack(data, ywCanvasNewText(wid, x + 20  * xdir,
				     y + 60 * ydir, txt));
    let head_threshold = 100
    if (ydir < 0) {
	head_threshold = -w
    } else {
	head_threshold = 20 * (1 + yeCountLines(txt)) + 40
    }
    yePushBack(data, ywCanvasNewCircleExt(wid, x + 120  * xdir,
					  y + head_threshold + 115, 100, "rgba: 255 255 255 50", 1));

    yePushBack(data, ywCanvasNewText(wid, x + 70  * xdir,
				     y + head_threshold, yeCreateString(CUR_HEAD)));
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
    yeCreateFunction(y_set_head, mod, "y_set_head")
    yeCreateFunction(y_set_talk_rect_style, mod, "y_set_talk_rect_style")
    ygRegistreFunc(4, "y_stop_head", "y_stop_head");
    ygRegistreFunc(1, "y_set_head", "y_set_head");
    ygRegistreFunc(2, "y_set_talk_rect_style", "y_set_talk_rect_style");
    ygRegistreFunc(4, "y_stop_helper", "y_stop_helper");
    return mod
}
