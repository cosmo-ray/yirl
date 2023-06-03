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

const DIR_RIGHT = 0
const DIR_LEFT = 1
const DIR_UP = 2
const DIR_DOWN = 3

const PC_POS_IDX = 0;
const PC_MOVER_IDX = 1
const PC_DROPSPEED_IDX = 2
const PC_TURN_CNT_IDX = 3
const PC_CANVAS_OBJ = 4
const PC_JMP_NUMBER = 5
const PC_HURT = 6
const PC_LIFE_ARRAY = 7
const PC_PUNCH_LIFE = 8
const PC_DIR = 9
const PC_PUNCH_OBJ = 10
const PC_PUNCH_MINFO = 11
const PC_DASH = 12

const MONSTER_STR_KEY = 0
const MONSTER_POS = 1
const MONSTER_MOVER = 2
const MONSTER_OBJ = 3
const MONSTER_ACC = 4
const MONSTER_DIR = 5

const BASE_SPEED = 16

const TYPE_WALL = 0
const TYPE_PC = 1
const TYPE_PIKE = 2
const TYPE_ANIMATION = 3
const TYPE_PUNCH = 4
const TYPE_MONSTER = 5
const TYPE_OBJ = 6

const CANVAS_OBJ_IDX = YCANVAS_UDATA_IDX + 1

const CANVAS_MONSTER_IDX = YCANVAS_UDATA_IDX + 1


const Y_MVER_X = 0
const Y_MVER_Y = 1
const Y_MVER_REST_X = 2
const Y_MVER_REST_Y = 3
const Y_MVER_LAST_X = 4
const Y_MVER_LAST_Y = 5
const Y_MVER_SPEEDUP = 6

function y_move_undo(pos, minfo)
{
    ywPosAddXY(pos, -yeGetIntAt(minfo, Y_MVER_LAST_X),
	       -yeGetIntAt(minfo, Y_MVER_LAST_Y))
}

function y_move_undo_x(pos, minfo)
{
    ywPosAddXY(pos, -yeGetIntAt(minfo, Y_MVER_LAST_X), 0)
}

function y_move_undo_y(pos, minfo)
{
    ywPosAddXY(pos, 0, -yeGetIntAt(minfo, Y_MVER_LAST_Y))
}

function y_move_pos(pos, minfo, turn_timer)
{
    let hremain = yeGetIntAt(minfo, Y_MVER_REST_X);
    let vremain = yeGetIntAt(minfo, Y_MVER_REST_Y);
    let speedup = yeGetFloatAtByIdx(minfo, Y_MVER_SPEEDUP)
    if (speedup == 0)
	speedup = 1
    let hms = yeGetIntAt(minfo, Y_MVER_X) * 100 * speedup + hremain;
    let vms = yeGetIntAt(minfo, Y_MVER_Y) * 100 * speedup + vremain;
    var mvx = 0;
    var mvy = 0;

    if (hms) {
	// mv = hms * turn_timer / 10000
	mvx = hms * turn_timer / 100000
	yeSetIntAt(minfo, Y_MVER_REST_X, mvx % 100)
	mvx = mvx / 100
    }

    if (vms) {
	mvy = vms * turn_timer / 100000
	yeSetIntAt(minfo, Y_MVER_REST_Y, mvy % 100)
	mvy = mvy / 100
    }
    yeSetIntAt(minfo, Y_MVER_LAST_X, mvx)
    yeSetIntAt(minfo, Y_MVER_LAST_Y, mvy)
    ywPosAddXY(pos, mvx, mvy)
}

function y_move_obj(o, minfo, turn_timer)
{
    y_move_pos(ywCanvasObjPos(o), minfo, turn_timer)
}

function y_move_x_speed(minfo)
{
    return yeGetIntAt(minfo, Y_MVER_X)
}

function y_move_y_speed(minfo)
{
    return yeGetIntAt(minfo, Y_MVER_Y)
}

function y_move_add_x_speed(minfo, to_Add)
{
    return yeAddAt(minfo, Y_MVER_X, to_Add)
}

function y_move_add_y_speed(minfo, to_Add)
{
    return yeAddAt(minfo, Y_MVER_Y, to_Add)
}

function y_move_last_x(minfo)
{
    return yeGetIntAt(minfo, Y_MVER_LAST_X)
}

function y_move_last_y(minfo)
{
    return yeGetIntAt(minfo, Y_MVER_LAST_Y)
}

function y_move_set_xspeed(minfo, xspeed)
{
    yeSetIntAt(minfo, Y_MVER_X, xspeed)
}

function y_move_set_yspeed(minfo, yspeed)
{
    yeSetIntAt(minfo, Y_MVER_Y, yspeed)
}

function y_mover_init(minfo)
{
    yeReCreateInt(0, minfo, "horizontal_speed");
    yeReCreateInt(0, minfo, "vertical_speed");
    yeReCreateInt(0, minfo, "horizontal_remain");
    yeReCreateInt(0, minfo, "vertical_remain");
    yeReCreateInt(0, minfo, "last_x");
    yeReCreateInt(0, minfo, "last_y");
    return minfo
}

function y_mover_new(father, name)
{
    let ret = yeCreateArray(father, name)
    return y_mover_init(ret)
}

function y_mover_new_at(father, name, at)
{
    let ret = yeCreateArrayAt(father, name, at)
    return y_mover_init(ret)
}

function print_life(wid, pc, pc_canel)
{
    var j = 0;
    let textures = yeGet(wid, "textures");
    let pj_pos = yeGet(pc_canel, PC_POS_IDX)
    let start_x = ywPosX(pj_pos) - 200
    let start_y = ywPosY(pj_pos) - 240
    let life_array = yeGet(pc_canel, PC_LIFE_ARRAY)

    ywCanvasClearArray(wid, life_array);
    for (var i = yeGetIntAt(pc, "life"); i > 0; i -= 5) {
	yePushBack(life_array,
		   ywCanvasNewImgFromTexture(wid, j * SPRITE_SIZE + start_x,
					     start_y,
					     yeGet(textures, "motivation")));
	++j;
    }
}

function move_punch(wid, pc_canel, turn_timer)
{
    var x_add = 0
    let pl = yeGetIntAt(pc_canel, PC_PUNCH_LIFE)


    if (pl != 0) {
	y_move_obj(yeGet(pc_canel, PC_PUNCH_OBJ),
		   yeGet(pc_canel, PC_PUNCH_MINFO), turn_timer)
    }

}

function print_all(wid)
{
    ywCanvasClear(wid);
    let map_a = yeGet(wid, "_m")
    let mi = yeGet(wid, "_mi")
    let pc = yeGet(wid, "pc")
    let pc_canel = yeGet(wid, "_pc")
    var sharp_str = yeGet(mi, "#")
    var objs = yeGet(mi, "objs")
    let textures = yeGet(wid, "textures");
    var map_real_size = yeGet(mi, "size")
    let monsters_info = yeGet(mi, "monsters")
    let monsters = yeGet(wid, "_monsters")

    var backgound = ywCanvasNewRectangle(wid, 0, 0, ywSizeW(map_real_size) * SPRITE_SIZE,
			 ywSizeH(map_real_size) * SPRITE_SIZE,
			 "rgba: 120 120 120 155")
    yeCreateIntAt(TYPE_ANIMATION, backgound, "amap-t", YCANVAS_UDATA_IDX)
    for (let i = 0; i < yeLen(map_a); ++i) {
	let s = yeGetStringAt(map_a, i)

	for (let j = 0; j < s.length; ++j) {
	    var c = s[j];

	    if (c == "'") {
		if (sharp_str)
		    ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
					 SPRITE_SIZE, SPRITE_SIZE / 3, "rgba: 0 0 0 255")

	    }

	    if (c == '#') {
		if (sharp_str)
		    ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
					 SPRITE_SIZE, SPRITE_SIZE, yeGetString(sharp_str))
		else
		    ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
					 SPRITE_SIZE, SPRITE_SIZE, "rgba: 0 0 0 255")
		continue;
		//print(i, j, c)
	    } else if (c == "^") {
		pike = ywCanvasNewImgFromTexture(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
						 yeGet(textures, "pike"))
		yeCreateIntAt(TYPE_PIKE, pike, "amap-t", YCANVAS_UDATA_IDX)

		continue;
	    }
	    var ic = parseInt(c)
	    if (!isNaN(ic)) {
		var object = yeGet(objs, ic)
		if (!object)
		    continue;
		var o = ywCanvasNewImgFromTexture(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
						  yeGet(textures, yeGetStringAt(object, 0)))
		yeCreateIntAt(TYPE_OBJ, o, "amap-t", YCANVAS_UDATA_IDX)
		yeCreateIntAt(ic, o, "objidx", CANVAS_OBJ_IDX)
		//yePrint(object)
	    }
	}
    }
    let pc_pos = yeGet(pc_canel, PC_POS_IDX)
    let pc_canvasobj = ywCanvasNewImgFromTexture(wid, ywPosX(pc_pos), ywPosY(pc_pos),
						 yeGet(textures, "guy-0"))
    // ywCanvasNewTextByStr(wid, ywPosX(pc_pos), ywPosY(pc_pos), " @ \n---")
    yeCreateIntAt(TYPE_PC, pc_canvasobj, "amap-t", YCANVAS_UDATA_IDX)
    ywCanvasObjReplacePos(pc_canvasobj, pc_pos)
    yePushAt2(pc_canel, pc_canvasobj, PC_CANVAS_OBJ)

    monsters.forEach(function(mon, idx) {
	let mon_key = yeGetStringAt(mon, 0)
	let mon_info = yeGet(monsters_info, mon_key)
	let mon_pos = yeGet(mon, 1)
	let canvasobj = ywCanvasNewImgFromTexture(wid, ywPosX(mon_pos), ywPosY(mon_pos),
						  yeGet(textures, yeGetStringAt(mon_info, "img")))
	// ywCanvasNewTextByStr(wid, ywPosX(pos), ywPosY(pos), " @ \n---")
	yeCreateIntAt(TYPE_MONSTER, canvasobj, "amap-t", YCANVAS_UDATA_IDX)
	yeCreateIntAt(idx, canvasobj, "mon_idx", CANVAS_MONSTER_IDX)
	ywCanvasObjReplacePos(canvasobj, mon_pos)
	yePushAt2(mon, canvasobj, MONSTER_OBJ)
    })
    print_life(wid, pc, pc_canel)
}

function amap_action(wid, events)
{
    let pc = yeGet(wid, "pc");
    let pc_stats = yeGet(pc, "stats")
    let pc_agility = yeGetIntAt(pc_stats, "agility")
    let mi = yeGet(wid, "_mi")
    let pc_canel = yeGet(wid, "_pc")
    let pc_pos = yeGet(pc_canel, PC_POS_IDX)
    let old_pos = yeCreateCopy(pc_pos)
    let pc_minfo = yeGet(pc_canel, PC_MOVER_IDX)
    let turn_timer = ywidGetTurnTimer()
    let monsters = yeGet(wid, "_monsters")
    let monsters_info = yeGet(mi, "monsters")
    var have_upkey = -1
    var ps_canvas_obj = yeGet(pc_canel, PC_CANVAS_OBJ)

    if (yevIsKeyUp(events, Y_LEFT_KEY)) {
	y_move_set_xspeed(pc_minfo, 0)
	have_upkey = DIR_LEFT
    } else if (yevIsKeyUp(events, Y_RIGHT_KEY)) {
	y_move_set_xspeed(pc_minfo, 0)
	have_upkey = DIR_RIGHT
    }

    if (yevIsKeyDown(events, Y_LEFT_KEY) && have_upkey != DIR_LEFT) {
	yeSetIntAt(pc_canel, PC_DIR, DIR_LEFT)
	y_move_set_xspeed(pc_minfo, -BASE_SPEED)
	ywCanvasHFlip(ps_canvas_obj);
    } else if (yevIsKeyDown(events, Y_RIGHT_KEY) && have_upkey != DIR_RIGHT) {
	yeSetIntAt(pc_canel, PC_DIR, DIR_RIGHT)
	y_move_set_xspeed(pc_minfo, BASE_SPEED)
	ywCanvasObjUnsetHFlip(ps_canvas_obj);
    } else if (yevIsKeyDown(events, Y_C_KEY) && yeGetIntAt(pc_canel, PC_DASH) == 0) {
	yeCreateFloatAt(2.5, pc_minfo, null, Y_MVER_SPEEDUP)
	yeSetIntAt(pc_canel, PC_DASH, 10)
    } else if (yevIsKeyDown(events, Y_X_KEY) && yeGetIntAt(pc_canel, PC_PUNCH_LIFE) == 0) {
	yeSetIntAt(pc_canel, PC_PUNCH_LIFE, 4 + pc_agility)
	let textures = yeGet(wid, "textures");
	let canvasobj = ywCanvasNewImgFromTexture(wid, ywPosX(pc_pos), ywPosY(pc_pos),
						  yeGet(textures, "punch"))

	if (yeGetIntAt(pc_canel, PC_DIR) == DIR_RIGHT) {
	    y_move_set_xspeed(yeGet(pc_canel, PC_PUNCH_MINFO), 30)
	} else {
	    ywCanvasHFlip(canvasobj);
	    y_move_set_xspeed(yeGet(pc_canel, PC_PUNCH_MINFO), -30)
	}

	// ywCanvasNewTextByStr(wid, ywPosX(pc_pos), ywPosY(pc_pos), " @ \n---")
	yePushAt2(pc_canel, canvasobj, PC_PUNCH_OBJ)
	yeCreateIntAt(TYPE_PUNCH, canvasobj, "amap-t", YCANVAS_UDATA_IDX)
    } else if (yevIsKeyDown(events, Y_SPACE_KEY) &&
	       yeGetIntAt(pc_canel, PC_JMP_NUMBER) < 2) {
	yeSetIntAt(pc_canel, PC_DROPSPEED_IDX, -25);
	yeAddAt(pc_canel, PC_JMP_NUMBER, 1);
    }

    if (yeGetIntAt(pc_canel, PC_TURN_CNT_IDX) > 20000) {
	var mult = yeGetIntAt(pc_canel, PC_TURN_CNT_IDX) / 20000

	if (yeGetIntAt(pc_canel, PC_DASH) > 0) {
	    yeAddAt(pc_canel, PC_DASH, -1 * mult)
	    if (yeGetIntAt(pc_canel, PC_DASH) == 0) {
		yeSetFloatAt(pc_minfo, Y_MVER_SPEEDUP, 1)
		yeSetIntAt(pc_canel, PC_DASH, -20)
	    }
	} else if (yeGetIntAt(pc_canel, PC_DASH) < 0) {
	    yeAddAt(pc_canel, PC_DASH, 1 * mult)
	}

	if (yeGetIntAt(pc_canel, PC_PUNCH_LIFE) > 0) {
	    yeAddAt(pc_canel, PC_PUNCH_LIFE, -1 * mult)
	    if (yeGetIntAt(pc_canel, PC_PUNCH_LIFE) == 0) {
		ywCanvasRemoveObj(wid, yeGet(pc_canel, PC_PUNCH_OBJ))
	    }
	}
	if (yeGetIntAt(pc_canel, PC_DASH) < 1) {
	    yeAddAt(pc_canel, PC_DROPSPEED_IDX, 2 * mult);
	}
	yeSetIntAt(pc_canel, PC_TURN_CNT_IDX, 0);
	if (yeGetIntAt(pc_canel, PC_HURT)) {
	    yeAddAt(pc_canel, PC_HURT, -1 * mult);
	}
    } else {
	yeAddAt(pc_canel, PC_TURN_CNT_IDX, turn_timer);
    }

    y_move_set_yspeed(pc_minfo, yeGetIntAt(pc_canel, PC_DROPSPEED_IDX));
    y_move_pos(pc_pos, pc_minfo, turn_timer);
    if (yeGetIntAt(pc_canel, PC_HURT) > 0) {
	print_life(wid, pc, pc_canel)
	return
    }
    let map_pixs_l = yeGet(wid, "map-pixs-l");
    var stop_fall = false;
    var stop_x = false;

    if (ywPosX(pc_pos) < 0 || ywPosX(pc_pos) + SPRITE_SIZE > ywSizeW(map_pixs_l))
	stop_x = true;
    if (ywPosY(pc_pos) > ywSizeH(map_pixs_l)) {
	print("you fall, wou lose !");
	ygCallFuncOrQuit(wid, "lose");
    } else if (yeGetIntAt(pc, "life") < 1) {
	print("no life left, wou lose !");
	ygCallFuncOrQuit(wid, "lose");
    }

    if (yeGetIntAt(pc_canel, PC_PUNCH_LIFE) > 0) {
	var punch_obj = yeGet(pc_canel, PC_PUNCH_OBJ)
	var cols = ywCanvasNewCollisionsArray(wid, punch_obj)

	if (cols) {
	    cols.forEach(function(c) {
		let ctype = yeGetIntAt(c, YCANVAS_UDATA_IDX)
		if (ctype == TYPE_MONSTER) {
		    if (ywCanvasObjectsCheckColisions(c, punch_obj)) {
			ywCanvasRemoveObj(wid, punch_obj)
			var mon_idx = yeGetIntAt(c, CANVAS_MONSTER_IDX)
			ywCanvasRemoveObj(wid, c)
			yeRemoveChildByIdx(monsters, mon_idx)
		    }
		}
	    })
	    yeDestroy(cols)
	}
    }

    var cols = ywCanvasNewCollisionsArray(wid, ps_canvas_obj)
    var direct_ret = false
    //yePrint(cols)
    if (cols) {
	cols.forEach(function(c) {
	    let ctype = yeGetIntAt(c, YCANVAS_UDATA_IDX)

	    if (ctype == TYPE_OBJ) {
		var objs = yeGet(mi, "objs");
		var action = yeGet(yeGet(objs, yeGetIntAt(c, CANVAS_OBJ_IDX)), 1);
		ywidAction(action, wid);
		direct_ret = true
		return true
	    } else if (ctype != TYPE_ANIMATION && ctype != TYPE_PUNCH) {
		if (ywCanvasObjectsCheckColisions(c, ps_canvas_obj)) {
		    if (ctype == TYPE_PIKE || ctype == TYPE_MONSTER) {
			yeSetIntAt(pc_canel, PC_HURT, 7);
			yeSetIntAt(pc_canel, PC_DROPSPEED_IDX, -25);
			yeAddAt(pc, "life", -5)
			print_life(wid, pc, pc_canel)
			return true
		    } else if ((ywPosY(old_pos) + SPRITE_SIZE - 1) <= ywPosY(ywCanvasObjPos(c))) {
			stop_fall = true
			print_life(wid, pc, pc_canel)
			return true
		    }
		}
	    }
	})
	yeDestroy(cols)
    }
    if (direct_ret)
	return

    if (stop_x)
	y_move_undo_x(pc_pos, pc_minfo)
    if (stop_fall) {
	y_move_undo_y(pc_pos, pc_minfo)
	yeSetIntAt(pc_canel, PC_JMP_NUMBER, 0);
	yeSetIntAt(pc_canel, PC_DROPSPEED_IDX, 0);
    }
    print_life(wid, pc, pc_canel)
    move_punch(wid, pc_canel, turn_timer)
    monsters.forEach(function(c, idx) {
	if (!c)
	    return;
	let tuple = yeCreateArray()
	let mon_key = yeGetStringAt(c, 0)
	let mon_info = yeGet(monsters_info, mon_key)

	yePushBack(tuple, c)
	yeCreateInt(turn_timer, tuple)
	ywidActions(wid, mon_info, tuple)
    })
}

function init_map(wid, map_str)
{
    var pc = yeGet(wid, "pc")
    let pc_canel = yeGet(wid, "_pc")
    yeReplaceBack(wid, ygFileToEnt(YJSON, map_str + ".json"), "_mi")
    let mi = yeGet(wid, "_mi")
    let parent_str = yeGet(mi, "parent")
    if (parent_str) {
	var parent = ygFileToEnt(YJSON, yeGetString(parent_str) + ".json");
	yePrint(parent);
	yeMergeInto(mi, parent, 0);
    }
    var map_str = ygFileToEnt(YRAW_FILE, map_str)
    var map_str_a = yeGetString(map_str).split('\n')
    var map_a = yeReCreateArray(wid, "_m")
    for (var l in map_str_a) {
	yeCreateString(map_str_a[l], map_a)
	let guy_index = map_str_a[l].indexOf('@')
	if (guy_index > -1) {
	    var pc_cam = yeGet(pc_canel, PC_POS_IDX)
	    ywPosSetInts(pc_cam, guy_index * SPRITE_SIZE, l * SPRITE_SIZE)
	    yeReplaceBack(wid, pc_cam, "cam")
	}
    }
    var size = yeGet(mi, "size")
    let map_pixs_l = ywSizeCreate(ywSizeW(size) * SPRITE_SIZE, ywSizeH(size) * SPRITE_SIZE)
    yeReplaceBack(wid, map_pixs_l, "map-pixs-l")
    let monsters = yeReCreateArray(wid, "_monsters")

    map_a.forEach(function(s, i) {
	s = yeGetString(s)
	for (let j = 0; j < s.length; ++j) {
	    var c = s[j];
	    c_char_code = c.charCodeAt(0);
	    if (c_char_code >= "a".charCodeAt(0) && c_char_code <= "z".charCodeAt(0)) {
		var mon = yeCreateArray(monsters, c)
		yeCreateString(c, mon) // MONSTER_STR_KEY 0
		ywPosCreate(j * SPRITE_SIZE, i * SPRITE_SIZE, mon) // MONSTER_POS 1
		y_mover_new(mon) // MONSTER_MOVER 2
	    }
	}

    })
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
		15, yaeInt(
		    15, pc,
		    "life"),
		"max_life"),
	    "xp")
	yeCreateString("Joe", pc, "name");
	var stats = yeCreateArray(pc, "stats");
	yeCreateInt(6, stats, "agility");
	yeCreateInt(4, stats, "strength");
    }
    //yePrint(pc)
    // canel for canvas element, it's the info about the screen position and stuff
    var pc_canel = yeCreateArray(wid, "_pc")

    let map = yeGetStringAt(wid, "map")

    // push at PC_POS_IDX (0) in pc_canel
    ywPosCreate(0, 0, pc_canel)
    y_mover_new(pc_canel) // create at PC_MOVER_IDX(1)
    yeCreateInt(0, pc_canel) // PC_DROPSPEED_IDX (2)
    yeCreateInt(0, pc_canel) // PC_TURN_CNT_IDX (3)

    ret = ywidNewWidget(wid, "canvas")

    let wid_pix = yeGet(wid, "wid-pix");
    let cam_t = ywPosCreate(ywRectW(wid_pix), ywRectH(wid_pix), wid, "cam-threshold")
    ywPosDoPercent(cam_t, 50);
    ywPosMultXY(cam_t, -1, -1);
    ywPosAddXY(cam_t, SPRITE_SIZE, 0);

    let size = ywSizeCreate(SPRITE_SIZE, SPRITE_SIZE);
    let textures = yeCreateArray(wid, "textures");
    ygModDir("amap");
    ywTextureNewImg("./door.png", null, textures, "door");
    ywTextureNewImg("./pike.png", null, textures, "pike");
    ywTextureNewImg("./gut-0.png", null, textures, "guy-0");
    ywTextureNewImg("./gut-1.png", null, textures, "guy-1");
    ywTextureNewImg("./punch.png", null, textures, "punch");
    ywTextureNewImg("./motivation.png", null, textures, "motivation");
    ywTextureNewImg("./uwu-head.png", null, textures, "uwu-head");
    ywTextureNewImg("./gamu.png", null, textures, "gamu");
    ywTextureNewImg("./vodeo-monster.png", null, textures, "vodeo-monster");
    ygModDirOut();

    yeCreateIntAt(0, pc_canel, "jmp-n", PC_JMP_NUMBER)
    yeCreateIntAt(0, pc_canel, "hurt", PC_HURT)
    yeCreateArrayAt(pc_canel, "life-array", PC_LIFE_ARRAY)
    yeCreateIntAt(0, pc_canel, "pl", PC_PUNCH_LIFE)
    yeCreateIntAt(0, pc_canel, "dash", PC_DASH)
    yeCreateIntAt(DIR_RIGHT, pc_canel, "dir", PC_DIR)
    y_mover_new_at(pc_canel, "p_minfo", PC_PUNCH_MINFO)
    init_map(wid, map)
    return ret
}

function win(wid)
{
    print("YOU WIN !!!");
    ygCallFuncOrQuit(wid, "win");
}

function next(wid)
{
    print("ACTION !!!!!")
    let mi = yeGet(wid, "_mi")
    yePrint(yeGet(mi, "next"))
    init_map(wid, yeGetStringAt(mi, "next"))
}

function monster_rand(wid, tuple)
{
    let mon = yeGet(tuple, 0)
    let turn_timer = yeGetIntAt(tuple, 1)
    let map_pixs_l = yeGet(wid, "map-pixs-l");
    let minfo = yeGet(mon, MONSTER_MOVER)

    if (!yeGet(mon, MONSTER_ACC)) {
	yeCreateIntAt(0, mon, "acc", MONSTER_ACC)
    }

    if (yeGetIntAt(mon, MONSTER_ACC) > 10000) {
	let rand = yuiRand() & 3
	let add_rand = 1 + (yuiRand() & 5)

	yeSetIntAt(mon, MONSTER_ACC, 0)

	if (rand == 0)
	    y_move_add_x_speed(minfo, add_rand)
	else if (rand == 1)
	    y_move_add_x_speed(minfo, -add_rand)
	else if (rand == 2)
	    y_move_add_y_speed(minfo, add_rand)
	else if (rand == 3)
	    y_move_add_y_speed(minfo, -add_rand)
    } else {
	yeAddAt(mon, MONSTER_ACC, turn_timer)
    }

    y_move_obj(yeGet(mon, MONSTER_OBJ), yeGet(mon, MONSTER_MOVER), turn_timer)
    let mon_pos = yeGet(mon, MONSTER_POS)
    if (ywPosX(mon_pos) < 0 || ywPosX(mon_pos) + SPRITE_SIZE > ywSizeW(map_pixs_l)) {
	y_move_undo_x(mon_pos, minfo)
	y_move_set_xspeed(minfo, -y_move_x_speed(minfo))
    }
    if (ywPosY(mon_pos) < 0 || ywPosY(mon_pos) + SPRITE_SIZE > ywSizeH(map_pixs_l)) {
	y_move_undo_y(mon_pos, minfo)
	y_move_set_yspeed(minfo, -y_move_y_speed(minfo))
    }
}

function monster_left_right(wid, tuple, distance)
{
    let mon = yeGet(tuple, 0)
    let turn_timer = yeGetIntAt(tuple, 1)
    let dist = yeGetInt(distance)

    if (!yeGet(mon, MONSTER_ACC)) {
	y_move_set_xspeed(yeGet(mon, MONSTER_MOVER), -25)
	yeCreateIntAt(0, mon, "acc", MONSTER_ACC)
	yeCreateIntAt(DIR_LEFT, mon, "dir", MONSTER_DIR)
    }

    y_move_obj(yeGet(mon, MONSTER_OBJ), yeGet(mon, MONSTER_MOVER), turn_timer)

    if (yeGetIntAt(mon, MONSTER_ACC) >= dist) {
	yeSetIntAt(mon, MONSTER_ACC, 0)
	if (yeGetIntAt(mon, MONSTER_DIR) == DIR_LEFT) {
	    y_move_set_xspeed(yeGet(mon, MONSTER_MOVER), 25)
	    yeSetIntAt(mon, MONSTER_DIR, DIR_RIGHT)
	} else {
	    y_move_set_xspeed(yeGet(mon, MONSTER_MOVER), -25)
	    yeSetIntAt(mon, MONSTER_DIR, DIR_LEFT)
	}
    } else {
	yeAddAt(mon, MONSTER_ACC, Math.abs(y_move_last_x(yeGet(mon, MONSTER_MOVER))))
    }
    //yePrint(mon)
    //yePrint(distance)
}

function monster_round(wid, tuple, distance)
{
    let mon = yeGet(tuple, 0)
    let turn_timer = yeGetIntAt(tuple, 1)
    let dist_x = yeGetIntAt(distance, 0)
    let dist_y = yeGetIntAt(distance, 1)

    if (!yeGet(mon, MONSTER_ACC)) {
	y_move_set_xspeed(yeGet(mon, MONSTER_MOVER), -25)
	yeCreateIntAt(0, mon, "acc", MONSTER_ACC)
	yeCreateIntAt(DIR_LEFT, mon, "dir", MONSTER_DIR)
    }

    y_move_obj(yeGet(mon, MONSTER_OBJ), yeGet(mon, MONSTER_MOVER), turn_timer)

    if (yeGetIntAt(mon, MONSTER_DIR) < DIR_UP) {
	if (yeGetIntAt(mon, MONSTER_ACC) >= dist_x) {
	    yeSetIntAt(mon, MONSTER_ACC, 0)
	    if (yeGetIntAt(mon, MONSTER_DIR) == DIR_LEFT) {
		y_move_set_xspeed(yeGet(mon, MONSTER_MOVER), 0)
		y_move_set_yspeed(yeGet(mon, MONSTER_MOVER), 25)
		yeSetIntAt(mon, MONSTER_DIR, DIR_DOWN)
	    } else {
		y_move_set_xspeed(yeGet(mon, MONSTER_MOVER), 0)
		y_move_set_yspeed(yeGet(mon, MONSTER_MOVER), -25)
		yeSetIntAt(mon, MONSTER_DIR, DIR_UP)
	    }
	} else {
	    yeAddAt(mon, MONSTER_ACC, Math.abs(y_move_last_x(yeGet(mon, MONSTER_MOVER))))
	}
    } else {
	if (yeGetIntAt(mon, MONSTER_ACC) >= dist_y) {
	    yeSetIntAt(mon, MONSTER_ACC, 0)
	    if (yeGetIntAt(mon, MONSTER_DIR) == DIR_DOWN) {
		y_move_set_yspeed(yeGet(mon, MONSTER_MOVER), 0)
		y_move_set_xspeed(yeGet(mon, MONSTER_MOVER), 25)
		yeSetIntAt(mon, MONSTER_DIR, DIR_RIGHT)
	    } else {
		y_move_set_yspeed(yeGet(mon, MONSTER_MOVER), 0)
		y_move_set_xspeed(yeGet(mon, MONSTER_MOVER), -25)
		yeSetIntAt(mon, MONSTER_DIR, DIR_LEFT)
	    }
	} else {
	    yeAddAt(mon, MONSTER_ACC, Math.abs(y_move_last_y(yeGet(mon, MONSTER_MOVER))))
	}
    }
    print("monster_round")
    yePrint(distance)
}

function mod_init(mod)
{
    ygInitWidgetModule(mod, "amap", yeCreateFunction("amap_init"))
    yeCreateString("lvl-test", yeGet(mod, "test_wid"), "map")
    yeCreateString("rgba: 255 255 255 255", yeGet(mod, "test_wid"), "background")
    yeCreateFunction(next, mod, "next")
    yeCreateFunction("win", mod, "win")
    let mons_mv = yeCreateArray(mod, "mons_mv")
    yeCreateFunction(monster_left_right, mons_mv, "left_right")
    yeCreateFunction(monster_rand, mons_mv, "rand")
    yeCreateFunction(monster_round, mons_mv, "round")
    return mod
}
