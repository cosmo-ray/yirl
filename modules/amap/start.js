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


const SPRITE_SIZE = 32;

const PC_POS_IDX = 0;
const PC_MOVER_IDX = 1


function y_move_undo(pos, minfo)
{
    ywPosAddXY(pos, -yeGetIntAt(minfo, 4), -yeGetIntAt(minfo, 5))
}

function y_move_pos(pos, minfo, turn_timer)
{
    let hremain = yeGetIntAt(minfo, 2);
    let vremain = yeGetIntAt(minfo, 3);
    let hms = yeGetIntAt(minfo, 0) * 100 + hremain;
    let vms = yeGetIntAt(minfo, 1) * 100 + vremain;
    var mvx = 0;
    var mvy = 0;

    if (hms) {
	// mv = hms * turn_timer / 10000
	mvx = hms * turn_timer / 100000
	print(mvx)
	yeSetIntAt(minfo, 2, mvx % 100)
	mvx = mvx / 100
    }

    if (vms) {
	mvy = vms * turn_timer / 1000000
	yeSetIntAt(minfo, 3, mv / 100)
	mvy = mvy / 100
    }
    print("VVVV")
    yePrint(minfo)
    print("^^^^")
    yeSetIntAt(minfo, 4, mvx)
    yeSetIntAt(minfo, 5, mvy)
    ywPosAddXY(pos, mvx, mvy)
}

function y_move_last_y(minfo)
{
    return yeGetIntAt(minfo, 5)
}

function y_move_last_x(minfo)
{
    return yeGetIntAt(minfo, 4)
}


function y_move_set_xspeed(minfo, xspeed)
{
    yeSetIntAt(minfo, 0, xspeed)
}

function y_move_set_yspeed(minfo, yspeed)
{
    yeSetIntAt(minfo, 0, yspeed)
}

function y_mover_new(father, name)
{
    let ret = yeCreateArray(father, name)
    yeCreateInt(0, ret, "horizontal_speed");
    yeCreateInt(0, ret, "vertical_speed");
    yeCreateInt(0, ret, "horizontal_remain");
    yeCreateInt(0, ret, "vertical_remain");
    yeCreateInt(0, ret, "last_x");
    yeCreateInt(0, ret, "last_y");
    return ret
}

function print_all(wid)
{
    ywCanvasClear(wid);
    let map_a = yeGet(wid, "_m")
    let mi = yeGet(wid, "_mi")
    let pc_canel = yeGet(wid, "_pc")

    for (let i = 0; i < yeLen(map_a); ++i) {
	let s = yeGetStringAt(map_a, i)

	for (let j = 0; j < s.length; ++j) {
	    c = s[j]
	    if (c == '#') {
		var sharp_str = yeGet(mi, "#")
		if (sharp_str)
		    ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
					 SPRITE_SIZE, SPRITE_SIZE, yeGetString(sharp_str))
		else
		    ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
					 SPRITE_SIZE, SPRITE_SIZE, "rgba: 0 0 0 255")
		//print(i, j, c)
	    } else if (c == "^") {
		ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
				     SPRITE_SIZE, SPRITE_SIZE, "rgba: 127 0 0 255")
	    }
	}
    }
    let pc_pos = yeGet(pc_canel, PC_POS_IDX)
    ywCanvasNewTextByStr(wid, ywPosX(pc_pos), ywPosY(pc_pos), " @ \n---")
}

function amap_action(wid, events)
{
    let pc_canel = yeGet(wid, "_pc")
    let pc_pos = yeGet(pc_canel, PC_POS_IDX)
    let pc_minfo = yeGet(pc_canel, PC_MOVER_IDX)
    let turn_timer = ywidGetTurnTimer()

    if (yevIsKeyUp(events, Y_LEFT_KEY) || yevIsKeyUp(events, Y_RIGHT_KEY)) {
	y_move_set_xspeed(pc_minfo, 0)
    }

    if (yevIsKeyDown(events, Y_LEFT_KEY)) {
	y_move_set_xspeed(pc_minfo, -14)
    } else if (yevIsKeyDown(events, Y_RIGHT_KEY)) {
	y_move_set_xspeed(pc_minfo, 14)
    }
    print("action !\n")
    y_move_pos(pc_pos, pc_minfo, turn_timer)
    map_pixs_l = yeGet(wid, "map-pixs-l");
    yePrint(map_pixs_l)
    if (ywPosX(pc_pos) < 0 || ywPosX(pc_pos) + SPRITE_SIZE > ywSizeW(map_pixs_l))
	y_move_undo(pc_pos, pc_minfo)
    print_all(wid)
}

function amap_init(wid)
{
    //yePrint(wid)
    ywSetTurnLengthOverwrite(-1)
    yeCreateFunction(amap_action, wid, "action")

    var pc = yeGet(wid, "pc")
    if (!pc) {
	pc = yeCreateArray(wid, "pc")
	yaeInt(
	    0, yaeInt(
		13, yaeInt(
		    13, pc,
		    "life"),
		"max_life"),
	    "xp")
	yeCreateString("Joe", pc, "name");
	var stats = yeCreateArray(pc, "stats");
	yeCreateInt(0, stats, "charm");
	yeCreateInt(4, stats, "smart");
	yeCreateInt(4, stats, "agility");
	yeCreateInt(4, stats, "strength");
    }
    yePrint(pc)
    // canel for canvas element, it's the info about the screen position and stuff
    var pc_canel = yeCreateArray(wid, "_pc")

    var map = yeGetStringAt(wid, "map")
    print(map + ".json")
    yePushBack(wid, ygFileToEnt(YJSON, map + ".json"), "_mi")
    let mi = yeGet(wid, "_mi")
    var map_str = ygFileToEnt(YRAW_FILE, map)
    var map_str_a = yeGetString(map_str).split('\n')
    var map_a = yeCreateArray(wid, "_m")
    for (var l in map_str_a) {
	yeCreateString(map_str_a[l], map_a)
	let guy_index = map_str_a[l].indexOf('@')
	if (guy_index > -1) {
	    // push at PC_POS_IDX (0) in pc_canel
	    var pc_cam = ywPosCreate(guy_index * SPRITE_SIZE, l * SPRITE_SIZE, pc_canel)
	    yeReplaceBack(wid, pc_cam, "cam")
	}
    }
    y_mover_new(pc_canel) // create at PC_MOVER_IDX(1)

    ret = ywidNewWidget(wid, "canvas")

    let wid_pix = yeGet(wid, "wid-pix");
    print("---------------VVVVV--------------------")
    let cam_t = ywPosCreate(ywRectW(wid_pix), ywRectH(wid_pix), wid, "cam-threshold")
    yePrint(cam_t)
    ywPosDoPercent(cam_t, 50);
    yePrint(cam_t)
    ywPosMultXY(cam_t, -1, -1);
    yePrint(cam_t)
    ywPosAddXY(cam_t, SPRITE_SIZE, 0);
    yePrint(cam_t)
    yePrint(map_a)
    yePrint(mi)
    print("--------------^^^^^^--------------------")
    let size = yeGet(mi, "size")
    ywSizeCreate(ywSizeW(size) * SPRITE_SIZE, ywSizeW(size) * SPRITE_SIZE, wid, "map-pixs-l")

    print_all(wid)
    return ret
}

function mod_init(mod)
{
    ygInitWidgetModule(mod, "amap", yeCreateFunction("amap_init"))
    yeCreateString("lvl-test", yeGet(mod, "test_wid"), "map")
    yeCreateString("rgba: 255 255 255 255", yeGet(mod, "test_wid"), "background")
    yePrint(mod)
    return mod
}
