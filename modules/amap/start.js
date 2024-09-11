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
const PC_HANDLER_OBJ = 4
const PC_JMP_NUMBER = 5
const PC_HURT = 6
const PC_LIFE_ARRAY = 7
const PC_PUNCH_LIFE = 8
const PC_DIR = 9
const PC_PUNCH_OBJ = 10
const PC_PUNCH_MINFO = 11
const PC_DASH = 12
const PC_NB_TURN_IDX = 13
const PC_PUNCH_COUNT_IDX = 14
/* jmp power allow you to jump longer if keep press 'space' */
const PC_JMP_POWER_LEFT = 15

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
const TYPE_BOSS = 7
const TYPE_BREAKABLE_BLOCK = 8
const TYPE_LIGHT_FLOOR = 9

const BOSS_OBJ = 0
const BOSS_MOVER = 1
const BOSS_TXT_LIVE = 2

const KEYDOWN_LEFT = 1
const KEYDOWN_RIGHT = 1 << 1
const KEYDOWN_UP = 1 << 2
const KEYDOWN_SPACE = 1 << 3

const CANVAS_OBJ_IDX = YCANVAS_UDATA_IDX + 1

const CANVAS_MONSTER_IDX = YCANVAS_UDATA_IDX + 1

const OBJECT_CANEL = 5
const OBJECT_MOVER = 6
const OBJECT_DIR = 7
const OBJECT_START_POS = 8

function print_life(wid, pc, pc_canel)
{
    let j = 0;
    let textures = yeGet(wid, "textures");
    let pj_pos = yeGet(pc_canel, PC_POS_IDX)
    let start_x = ywPosX(pj_pos) - 200
    let start_y = ywPosY(pj_pos) - 240
    let life_array = yeGet(pc_canel, PC_LIFE_ARRAY)

    ywCanvasClearArray(wid, life_array);
    if (wid.geti("life-bar")) {
	const max_life = pc.geti("max_life")
	const life = pc.geti("life")

	start_y += 20
	life_array.push(ywCanvasNewTextByStr(wid, start_x, start_y, "life:"))
	start_x += 60
	life_array.push(ywCanvasNewRectangle(wid, start_x, start_y, max_life * 2 + 10, 16,
					     "rgba: 100 100 100 255"))
	life_array.push(ywCanvasNewRectangle(wid, start_x + 5, start_y + 3,
					     life * 2, 10,
					     "rgba: 120 220 120 255"))
    } else {
	for (let i = yeGetIntAt(pc, "life"); i > 0; i -= 5) {
	    yePushBack(life_array,
		       ywCanvasNewImgFromTexture(wid, j * SPRITE_SIZE + start_x,
						 start_y,
						 yeGet(textures, "motivation")));
	    ++j;
	}
    }

    let next_lvl = wid.geti("next-lvl")
    if (next_lvl > 0) {
	let cur_xp = pc.geti("xp")

	start_y += 20
	start_x -= 60
	life_array.push(ywCanvasNewTextByStr(wid, start_x, start_y, "xp:"))
	start_x += 60
	life_array.push(ywCanvasNewRectangle(wid, start_x, start_y, next_lvl * 2 + 10, 16,
					     "rgba: 100 100 100 255"))
	life_array.push(ywCanvasNewRectangle(wid, start_x + 5, start_y + 3,
					     cur_xp * 2, 10,
					     "rgba: 120 220 120 255"))

    }
}

function move_punch(wid, pc_canel, turn_timer)
{
    let pl = yeGetIntAt(pc_canel, PC_PUNCH_LIFE)


    if (pl != 0) {
	y_move_obj(yeGet(pc_canel, PC_PUNCH_OBJ),
		   yeGet(pc_canel, PC_PUNCH_MINFO), turn_timer)
    }

}

function yamap_push_animation(wid, pos, texture, lifetime, end_callback)
{
    let anims = yeTryCreateArray(wid, "animations")
    if (yeType(texture) == YSTRING) {
	let textures = wid.get("textures")
	texture = yeGet(textures, texture)
    }

    a = ywCanvasNewImgFromTexture(wid, ywPosX(pos), ywPosY(pos), texture)
    yeCreateIntAt(TYPE_ANIMATION, a, "amap-t", YCANVAS_UDATA_IDX)
    let anim_info = yeCreateArray(anims)
    anim_info.setAt(0, a)
    anim_info.setAt(1, lifetime)
    if (end_callback)
	anim_info.setAt(2, end_callback)
    return anim_info
}

function yamap_push_obj(wid, pos, idx)
{
    let ic = idx
    let mi = yeGet(wid, "_mi")
    let objs = yeGet(mi, "objs")
    let object = yeGet(objs, ic)
    let textures = wid.get("textures")
    let o = ywCanvasNewImgFromTexture(wid, ywPosX(pos) * SPRITE_SIZE,
				      ywPosY(pos) * SPRITE_SIZE,
				      yeGet(textures, yeGetStringAt(object, 0)))
    yeCreateIntAt(TYPE_OBJ, o, "amap-t", YCANVAS_UDATA_IDX)
    yeCreateIntAt(ic, o, "objidx", CANVAS_OBJ_IDX)
    object.setAt(OBJECT_CANEL, o)
}

function yamap_push_monster(wid, pos, type)
{
    let monsters = wid.get("_monsters")
    let textures = wid.get("textures")
    let monster_info = wid.get("_mi").get("monsters")
    let mon = yeCreateArray(monsters)
    yeCreateString("p", mon)
    yeCreateCopy(pos, mon, "pos")
    y_mover_new(mon, "mover")

    yamap_generate_monster_canvasobj(wid, textures, mon, monster_info,
				     yeLen(monsters) - 1)
    return mon
}

function yamap_generate_monster_canvasobj(wid, textures,
					  mon, monsters_info, idx)
{
    let mon_key = yeGetStringAt(mon, 0)
    let mon_info = yeGet(monsters_info, mon_key)
    let mon_pos = yeGet(mon, 1)
    let canvasobj = ywCanvasNewImgFromTexture(wid, ywPosX(mon_pos), ywPosY(mon_pos),
					      yeGet(textures, yeGetStringAt(mon_info, "img")))

    yeCreateIntAt(TYPE_MONSTER, canvasobj, "amap-t", YCANVAS_UDATA_IDX)
    yeCreateIntAt(idx, canvasobj, "mon_idx", CANVAS_MONSTER_IDX)
    ywCanvasObjReplacePos(canvasobj, mon_pos)
    yePushAt2(mon, canvasobj, MONSTER_OBJ)
}

function print_all(wid)
{
    ywCanvasClear(wid);
    let map_a = yeGet(wid, "_m")
    let mi = yeGet(wid, "_mi")
    let pc = yeGet(wid, "pc")
    let pc_canel = yeGet(wid, "_pc")
    let sharp_str = yeGet(mi, "#")
    let upsharp_str = yeGet(mi, "'")
    let brekable_str = yeGet(mi, "=")
    let objs = yeGet(mi, "objs")
    let objs_conditions = mi.get("objs_condition")
    let textures = yeGet(wid, "textures");
    let map_real_size = yeGet(mi, "size")
    let monsters_info = yeGet(mi, "monsters")
    let monsters = yeGet(wid, "_monsters")

    let backgound = ywCanvasNewRectangle(wid, 0, 0, ywSizeW(map_real_size) * SPRITE_SIZE,
			 ywSizeH(map_real_size) * SPRITE_SIZE,
			 "rgba: 120 120 120 155")
    yeCreateIntAt(TYPE_ANIMATION, backgound, "amap-t", YCANVAS_UDATA_IDX)
    for (let i = 0; i < yeLen(map_a); ++i) {
	let s = yeGetStringAt(map_a, i)

	for (let j = 0; j < s.length; ++j) {
	    let c = s[j];

	    if (c == "'") {
		let floor = null
		if (upsharp_str)
		    floor = ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
						 SPRITE_SIZE, SPRITE_SIZE / 3, upsharp_str.s())
		else
		    floor = ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
						 SPRITE_SIZE, SPRITE_SIZE / 3,
						 "rgba: 0 0 0 255")

		yeCreateIntAt(TYPE_LIGHT_FLOOR, floor, "amap-t", YCANVAS_UDATA_IDX)
		continue;
	    }

	    if (c == '=') {
		let breakable = ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
				     SPRITE_SIZE, SPRITE_SIZE / 2, "rgba: 100 200 100 255")
		yeCreateIntAt(TYPE_BREAKABLE_BLOCK, breakable, "amap-t", YCANVAS_UDATA_IDX)
		continue;

	    }
	    else if (c == '#') {
		if (sharp_str)
		    ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
					 SPRITE_SIZE, SPRITE_SIZE, yeGetString(sharp_str))
		else
		    ywCanvasNewRectangle(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
					 SPRITE_SIZE, SPRITE_SIZE, "rgba: 0 0 0 255")
		continue;
		//print(i, j, c)
	    } else if (c == "^") {
		let pike = ywCanvasNewImgFromTexture(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
						 yeGet(textures, "pike"))
		yeCreateIntAt(TYPE_PIKE, pike, "amap-t", YCANVAS_UDATA_IDX)

		continue;
	    }
	    let ic = parseInt(c)
	    if (!isNaN(ic)) {
		let object = yeGet(objs, ic)
		if (!object)
		    continue;
		if (objs_conditions) {
		    let this_condition = objs_conditions.get(ic)

		    if (this_condition && !yeCheckCondition(this_condition))
			continue;
		}
		let o = ywCanvasNewImgFromTexture(wid, j * SPRITE_SIZE, i * SPRITE_SIZE,
						  yeGet(textures, yeGetStringAt(object, 0)))
		yeCreateIntAt(TYPE_OBJ, o, "amap-t", YCANVAS_UDATA_IDX)
		yeCreateIntAt(ic, o, "objidx", CANVAS_OBJ_IDX)
		object.setAt(OBJECT_CANEL, o)
	    }
	}
    }
    let pc_pos = yeGet(pc_canel, PC_POS_IDX)
    let pc_handler = yeGet(wid, "pc_handler")
    yGenericHandlerRefresh(pc_handler)
    let pc_canvasobj = yGenericCurCanvas(pc_handler)
    yeCreateIntAt(TYPE_PC, pc_canvasobj, "amap-t", YCANVAS_UDATA_IDX)

    // ywCanvasNewImgFromTexture(wid, ywPosX(pc_pos), ywPosY(pc_pos), yeGet(textures, "guy-0"))
    // ywCanvasNewTextByStr(wid, ywPosX(pc_pos), ywPosY(pc_pos), " @ \n---")
    yGenericUsePos(pc_handler, pc_pos)
    yePushAt2(pc_canel, pc_handler, PC_HANDLER_OBJ)

    monsters.forEach(function(mon, idx) {
	yamap_generate_monster_canvasobj(wid, textures, mon, monsters_info, idx)
    })
    print_life(wid, pc, pc_canel)
}

function amap_action(wid, events)
{
    let pc = yeGet(wid, "pc");
    let pc_stats = yeGet(pc, "stats")
    let pc_agility = yeGetIntAt(pc_stats, "agility")
    let pc_strength = yeGetIntAt(pc_stats, "strength")
    let mi = yeGet(wid, "_mi")
    let pc_canel = yeGet(wid, "_pc")
    let boss = yeGet(wid, "_boss")
    let hooks = yeGet(mi, "hooks")

    let pc_minfo = yeGet(pc_canel, PC_MOVER_IDX)
    if (hooks) {
	let hooks_ret = false;

	hooks.forEach(function (h, idx) {
	    if (!h)
		return false
	    let at = yeGet(h, "at")
	    if (yeGetInt(at) == yeGetIntAt(pc_canel, PC_NB_TURN_IDX)) {
		if (h.getb("stop movement")) {
		    y_move_set_xspeed(pc_minfo, 0)
		}
		ywidActions(wid, h)
		hooks_ret = true;
		yeRemoveChildByIdx(hooks, idx)
		return true
	    }
	})
	if (hooks_ret)
	    return;
    }

    let pc_pos = yeGet(pc_canel, PC_POS_IDX)
    let old_pos = yeCreateCopy(pc_pos)
    let turn_timer = ywidGetTurnTimer()
    let monsters = yeGet(wid, "_monsters")
    let monsters_info = yeGet(mi, "monsters")
    let have_upkey = -1
    let pc_handler = yeGet(pc_canel, PC_HANDLER_OBJ)

    let on = wid.get("on")
    if (on) {
	const on_len = yeLen(on)
	for (let i = 0; i < on_len; ++i) {
	    let key = yeGetKeyAt(on, i)
	    if (key == "esc" && yevIsKeyDown(events, Y_ESC_KEY)) {
		on.get(i).call(wid)
	    }

	}
    }
    if (yevIsKeyUp(events, Y_LEFT_KEY)) {
	y_move_set_xspeed(pc_minfo, 0)
	have_upkey = DIR_LEFT
	wid.setAt("keydown", wid.geti("keydown") & (~KEYDOWN_LEFT))
    } else if (yevIsKeyUp(events, Y_RIGHT_KEY)) {
	y_move_set_xspeed(pc_minfo, 0)
	have_upkey = DIR_RIGHT
	wid.setAt("keydown", wid.geti("keydown") & (~KEYDOWN_RIGHT))
    }

    if (yevIsKeyUp(events, Y_UP_KEY)) {
	wid.setAt("keydown", wid.geti("keydown") & (~KEYDOWN_UP))
    } else if (yevIsKeyDown(events, Y_UP_KEY)) {
	wid.setAt("keydown", wid.geti("keydown") | KEYDOWN_UP)
    }

    if (yevIsKeyUp(events, Y_SPACE_KEY)) {
	wid.setAt("keydown", wid.geti("keydown") & (~KEYDOWN_SPACE))
    }

    if (yevIsKeyDown(events, Y_LEFT_KEY) && have_upkey != DIR_LEFT) {
	yeSetIntAt(pc_canel, PC_DIR, DIR_LEFT)
	y_move_set_xspeed(pc_minfo, -BASE_SPEED)
	wid.setAt("keydown", wid.geti("keydown") | KEYDOWN_LEFT)
	pc_handler.setAt("flip", 1)
    } else if (yevIsKeyDown(events, Y_RIGHT_KEY) && have_upkey != DIR_RIGHT) {
	yeSetIntAt(pc_canel, PC_DIR, DIR_RIGHT)
	wid.setAt("keydown", wid.geti("keydown") | KEYDOWN_RIGHT)
	y_move_set_xspeed(pc_minfo, BASE_SPEED)
	pc_handler.setAt("flip", 0)
    } else if (yevIsKeyDown(events, Y_C_KEY) && yeGetIntAt(pc_canel, PC_DASH) == 0) {
	let dir = 1
	y_move_set_xspeed(pc_minfo, BASE_SPEED)
	if (pc_handler.geti("flip")) {
	    dir = -1
	}
	y_move_set_yspeed(pc_minfo, 0)
	y_move_set_xspeed(pc_minfo, BASE_SPEED * dir)
	yeCreateFloatAt(3, pc_minfo, null, Y_MVER_SPEEDUP)
	yeSetIntAt(pc_canel, PC_DASH, 10)
    } else if (yevIsKeyDown(events, Y_X_KEY) && yeGetIntAt(pc_canel, PC_PUNCH_LIFE) == 0) {
	yeSetIntAt(pc_canel, PC_PUNCH_LIFE, 4 + pc_strength / 3)
	let textures = yeGet(wid, "textures");
	let canvasobj = ywCanvasNewImgFromTexture(wid, ywPosX(pc_pos), ywPosY(pc_pos),
						  yeGet(textures, "punch"))
	let atk_size = wid.gets("attack-sprite-size")
	if (atk_size == "half") {
	    let canvasobj_size = ywCanvasObjSize(wid, canvasobj)
	    ywCanvasForceSize(canvasobj, ywSizeCreate(ywSizeW(canvasobj_size) / 2,
						      ywSizeH(canvasobj_size) / 2))
	}
	if (wid.get("attack-sprite-threshold")) {
	    ywCanvasMoveObj(canvasobj, wid.get("attack-sprite-threshold"))
	}

	let dash_val = 0
	let base_cnt = 5 + pc_agility / 10
	const dash = pc_canel.geti(PC_DASH)
	if (dash > 0) {
	    dash_val = 30 * dash / 10 + 10
	    base_cnt = base_cnt / 2
	}
	let base_speed = 25 + pc_agility + dash_val

	let kd = wid.geti("keydown")
	y_move_set_yspeed(yeGet(pc_canel, PC_PUNCH_MINFO), 0)
	if (wid.geti("can_upshoot") && kd != 0) {
	    y_move_set_xspeed(yeGet(pc_canel, PC_PUNCH_MINFO), 0)

	    if (kd & KEYDOWN_UP) {
		ywCanvasRotate(canvasobj, -90)
		y_move_set_yspeed(yeGet(pc_canel, PC_PUNCH_MINFO), -base_speed)
	    }

	    if (kd & KEYDOWN_LEFT) {
		ywCanvasHFlip(canvasobj);
		if (kd & KEYDOWN_UP)
		    ywCanvasRotate(canvasobj, 45)
		y_move_set_xspeed(yeGet(pc_canel, PC_PUNCH_MINFO), -base_speed)
	    } else if (kd & KEYDOWN_RIGHT) {
		if (kd & KEYDOWN_UP)
		    ywCanvasRotate(canvasobj, -45)
		ywCanvasMoveObjXY(canvasobj, 20, 0)
		y_move_set_xspeed(yeGet(pc_canel, PC_PUNCH_MINFO), base_speed)
	    } else {
		ywCanvasMoveObjXY(canvasobj, 10, 0)
	    }
	} else {
	    if (yeGetIntAt(pc_canel, PC_DIR) == DIR_RIGHT) {
		ywCanvasMoveObjXY(canvasobj, 20, 0)
		y_move_set_xspeed(yeGet(pc_canel, PC_PUNCH_MINFO), base_speed)
	    } else {
		ywCanvasHFlip(canvasobj);
		y_move_set_xspeed(yeGet(pc_canel, PC_PUNCH_MINFO), -base_speed)
	    }
	}
	pc_canel.setAt(PC_PUNCH_COUNT_IDX, base_cnt)

	// ywCanvasNewTextByStr(wid, ywPosX(pc_pos), ywPosY(pc_pos), " @ \n---")
	yePushAt2(pc_canel, canvasobj, PC_PUNCH_OBJ)
	yeCreateIntAt(TYPE_PUNCH, canvasobj, "amap-t", YCANVAS_UDATA_IDX)
    } else if (yevIsKeyDown(events, Y_SPACE_KEY) &&
	       yeGetIntAt(pc_canel, PC_JMP_NUMBER) < 2) {
	yeSetIntAt(pc_canel, PC_DROPSPEED_IDX, -25);
	yeAddAt(pc_canel, PC_JMP_NUMBER, 1);
	wid.setAt("keydown", wid.geti("keydown") | KEYDOWN_SPACE)
	pc_canel.setAt(PC_JMP_POWER_LEFT, pc.geti("jmp-power"))
    }

    if (yeGetIntAt(pc_canel, PC_TURN_CNT_IDX) > 20000) {
	let walk = false
	if (pc_canel.geti(PC_DASH) > 0) {
	    yGenericTextureArraySet(pc_handler, "dash")
	} else if (yeGetIntAt(pc_canel, PC_PUNCH_COUNT_IDX) > 0) {
	    yGenericTextureArraySet(pc_handler, "punch")
	    yeAddAt(pc_canel, PC_PUNCH_COUNT_IDX, -1);
	} else if (yeGetIntAt(pc_canel, PC_DROPSPEED_IDX) < 0) {
	    yGenericTextureArraySet(pc_handler, "jmp")
	} else {
	    yGenericTextureArraySet(pc_handler, "base")
	    walk = true
	}
	if ((pc_canel.geti(PC_NB_TURN_IDX) % 3) == 0 && y_move_x_speed(pc_minfo) != 0) {
	    yGenericNext(pc_handler)
	    if (walk && (pc_canel.geti(PC_NB_TURN_IDX) % 6) == 0) {
		if (wid.get("_walk_sound")) {
		    ySoundPlay(wid.geti("_walk_sound"))
		}
	    }
	}
	yGenericHandlerRefresh(pc_handler)
	let pc_canvasobj = yGenericCurCanvas(pc_handler)
	yeCreateIntAt(TYPE_PC, pc_canvasobj, "amap-t", YCANVAS_UDATA_IDX)

	let mult = yeGetIntAt(pc_canel, PC_TURN_CNT_IDX) / 20000

	yeAddAt(pc_canel, PC_NB_TURN_IDX, 1)
	if (yeGetIntAt(pc_canel, PC_DASH) > 0) {
	    yeAddAt(pc_canel, PC_DASH, -1 * mult)
	    if (yeGetIntAt(pc_canel, PC_DASH) == 0) {
		yeSetFloatAt(pc_minfo, Y_MVER_SPEEDUP, 1)
		yeSetIntAt(pc_canel, PC_DASH, -20)
		if (!wid.geti("keydown"))
		    y_move_set_xspeed(pc_minfo, 0)
	    }
	} else if (yeGetIntAt(pc_canel, PC_DASH) < 0) {
	    yeAddAt(pc_canel, PC_DASH, 1 * mult)
	}

	if (yeGetIntAt(pc_canel, PC_PUNCH_LIFE) > 0) {
	    yeAddAt(pc_canel, PC_PUNCH_LIFE, -1 * mult)
	    if (yeGetIntAt(pc_canel, PC_PUNCH_LIFE) <= 0) {
		ywCanvasRemoveObj(wid, yeGet(pc_canel, PC_PUNCH_OBJ))
		yeSetIntAt(pc_canel, PC_PUNCH_LIFE, 0)
	    }
	}
	/* can't jump in dash */
	if (yeGetIntAt(pc_canel, PC_DASH) < 1) {
	    let kd = wid.geti("keydown")

	    if (pc_canel.geti(PC_JMP_POWER_LEFT) > 0 && kd & KEYDOWN_SPACE) {
		yeAddAt(pc_canel, PC_JMP_POWER_LEFT, -2 * mult);
	    } else {
		yeAddAt(pc_canel, PC_DROPSPEED_IDX, 2 * mult);
	    }
	}
	yeSetIntAt(pc_canel, PC_TURN_CNT_IDX, 0);
	if (yeGetIntAt(pc_canel, PC_HURT)) {
	    yeAddAt(pc_canel, PC_HURT, -1 * mult);
	}
    } else {
	yeAddAt(pc_canel, PC_TURN_CNT_IDX, turn_timer);
    }

    if (pc_canel.geti(PC_DASH) < 1) {
	y_move_set_yspeed(pc_minfo, yeGetIntAt(pc_canel, PC_DROPSPEED_IDX));
    }
    y_move_pos(pc_pos, pc_minfo, turn_timer);
    if (yeGetIntAt(pc_canel, PC_HURT) > 0) {
	print_life(wid, pc, pc_canel)
	return
    } else {
	pc_handler.rm("colorMod")
    }
    let map_pixs_l = yeGet(wid, "map-pixs-l");
    let stop_fall = false;
    let stop_x = false;

    if (ywPosX(pc_pos) < 0 || ywPosX(pc_pos) + SPRITE_SIZE > ywSizeW(map_pixs_l))
	stop_x = true;
    if (wid.geti("block-up") > 0 && ywPosY(pc_pos) < 0) {
	if (pc_canel.geti(PC_DROPSPEED_IDX) < 0)
	    yeSetIntAt(pc_canel, PC_DROPSPEED_IDX, 0);
	pc_canel.setAt(PC_JMP_POWER_LEFT, 0)
    }
    if (ywPosY(pc_pos) > ywSizeH(map_pixs_l)) {
	print("you fall, wou lose !");
	ygCallFuncOrQuit(wid, "lose");
    } else if (yeGetIntAt(pc, "life") < 1) {
	print("no life left, wou lose !");
	ygCallFuncOrQuit(wid, "lose");
    }

    let next_lvl = wid.geti("next-lvl")
    if (next_lvl > 0 && pc.geti("xp") >= next_lvl) {
	wid.get("lvl_up").call(wid)
    }


    if (boss) {
	function boss_call() {
	    let boss_i = yeGet(mi, "boss")
	    let life = yeGetIntAt(boss_i, "life")

	    let tuple = yeCreateArray()
	    yePushBack(tuple, boss)
	    yeCreateInt(turn_timer, tuple)

	    if (life < 1) {
		let ret = ywidAction(yeGet(boss_i, "win"), wid, tuple)
		if (ret & 0x10)
		    y_move_set_xspeed(pc_minfo, 0)
		ret = ret & 0x0f
		if (ret == 1) {
		    return 1
		} else if (ret == 2) {
		    ywCanvasRemoveObj(wid, boss.get(0))
		    mi.rm("boss")
		    wid.rm("_boss")
		    let txt_obj = yeGet(boss, BOSS_TXT_LIVE)
		    ywCanvasRemoveObj(wid, txt_obj)
		}
		return 0;
	    }
	    let txt_start_x = ywPosX(old_pos)
	    let txt_start_y = ywPosY(old_pos) - 220

	    ywidActions(wid, boss_i, tuple)

	    let txt_obj = yeGet(boss, BOSS_TXT_LIVE)
	    ywCanvasObjSetPos(txt_obj, txt_start_x, txt_start_y)
	    let base_txt = "BOSS life: |"
	    for (; life > 0; life -= 5) {
		base_txt = base_txt + "="
	    }
	    base_txt = base_txt + "|"
	    ywCanvasStringSet(txt_obj, yeCreateString(base_txt))
	    return 0
	}
	if (boss_call()) {
	    return
	}
    }

    if (yeGetIntAt(pc_canel, PC_PUNCH_LIFE) > 0) {
	let punch_obj = yeGet(pc_canel, PC_PUNCH_OBJ)
	let cols = ywCanvasNewProjectedCollisionsArrayExt(wid, punch_obj, null, null, null)

	if (cols) {
	    let monsters_info = yeGet(mi, "monsters")

	    for (c of cols) {
		let ctype = yeGetIntAt(c, YCANVAS_UDATA_IDX)
		if (ctype == TYPE_MONSTER) {
		    if (ywCanvasObjectsCheckColisions(c, punch_obj)) {
			let mon_idx = yeGetIntAt(c, CANVAS_MONSTER_IDX)
			yeAddAt(pc_canel, PC_PUNCH_LIFE, -2)
			let next_lvl = wid.geti("next-lvl")
			if (next_lvl) {
			    pc.get("xp").add(1)
			}
			let mon = monsters.get(mon_idx)
			let mon_info = monsters_info.get(mon.gets(0))
			if (mon_info.get("dead")) {
			    ygGet(mon_info.gets("dead")).call(wid, mon, mon_info)
			}
			ywCanvasRemoveObj(wid, c)
			yeRemoveChildByIdx(monsters, mon_idx)

		    }
		} else if (ctype == TYPE_BREAKABLE_BLOCK) {
		    ywCanvasRemoveObj(wid, c)
		    yeAddAt(pc_canel, PC_PUNCH_LIFE, -25)
		} else if (ctype == TYPE_BOSS) {
		    let boss_i = yeGet(mi, "boss")
		    let dmg = pc_strength

		    yeAddAt(boss_i, "life", -dmg)
		    yeAddAt(pc_canel, PC_PUNCH_LIFE, -25)
		    ywCanvasSetColorModRGBA(boss.get(0), 255, 100, 0, 255)
		    wid.setAt("boss-h-timer", 100000)
		    have_boss_take_dmg = true
		}

		if (yeGetIntAt(pc_canel, PC_PUNCH_LIFE) <= 0) {
		    ywCanvasRemoveObj(wid, yeGet(pc_canel, PC_PUNCH_OBJ))
		    yeSetIntAt(pc_canel, PC_PUNCH_LIFE, 0)
		    return true
		}

	    }
	    yeDestroy(cols)
	}
    } else if (boss) {
	let boss_t = wid.geti("boss-h-timer")
	if (boss_t > 0) {
	    boss_t -= turn_timer
	    if (boss_t <= 0) {
		ywCanvasRemoveColorMod(boss.get(0));
		wid.rm("boss-h-timer")
	    } else {
		wid.setAt("boss-h-timer", boss_t)
	    }
	}
    }

    let movable_objs = mi.get("move_objs")
    if (movable_objs) {
	let objs = yeGet(mi, "objs");

	for (o_info of movable_objs) {
	    let o = objs.get(o_info.i())
	    if (!o)
		continue;
	    let o_canel = o.get(OBJECT_CANEL)
	    if (!o_canel)
		continue;
	    let o_pos = ywCanvasObjPos(o_canel)
	    let mover = o.get(OBJECT_MOVER)
	    if (!mover) {
		mover = y_mover_new_at(o, "mv", OBJECT_MOVER)
		o.setAt(OBJECT_START_POS, yeCreateCopy(o_pos))
		y_move_set_yspeed(mover, -5)
	    }
	    const pos_diff = ywPosY(o_pos) - ywPosY(o.get(OBJECT_START_POS))
	    if (pos_diff > 4)
		y_move_set_yspeed(mover, -5)
	    else if (pos_diff < -4)
		y_move_set_yspeed(mover, 5)
	    y_move_obj(o_canel, mover, turn_timer)
	}
    }

    let pc_canvas_obj = yGenericCurCanvas(pc_handler)
    let cols = ywCanvasNewCollisionsArray(wid, pc_canvas_obj)
    let direct_ret = false
    //yePrint(cols)
    if (cols) {
	cols.forEach(function(c) {
	    let ctype = yeGetIntAt(c, YCANVAS_UDATA_IDX)

	    if (ctype == TYPE_OBJ) {
		let objs = yeGet(mi, "objs");
		let action = yeGet(yeGet(objs, yeGetIntAt(c, CANVAS_OBJ_IDX)), 1);

		let ret = ywidAction(action, wid);
		if (ret & 0x10)
		    y_move_set_xspeed(pc_minfo, 0)
		ret = ret & 0x0f

		if (ret == 1) {
		    direct_ret = true
		    return true
		}
		if (ret == 2) {
		    const idx = c.geti("objidx")
		    ywCanvasRemoveObj(wid, c)
		    objs.rm(idx)
		}
	    } else if (ctype == TYPE_BREAKABLE_BLOCK) {
		stop_x = true
		if ((ywPosY(old_pos) + SPRITE_SIZE) <= ywPosY(ywCanvasObjPos(c))) {
		    stop_fall = true
		    print_life(wid, pc, pc_canel)
		}
		if (pc_canel.geti(PC_DROPSPEED_IDX) < 0)
		    pc_canel.setAt(PC_DROPSPEED_IDX, 0)
		return false;
	    } else if (ctype != TYPE_ANIMATION && ctype != TYPE_PUNCH) {
		if (ywCanvasObjectsCheckColisions(c, pc_canvas_obj)) {
		    if (ctype == TYPE_PIKE || ctype == TYPE_MONSTER || ctype == TYPE_BOSS) {
			if (yeGetIntAt(pc_canel, PC_HURT) == 0) {
			    yeAddAt(pc, "life", -5)
			    pc_handler.setAt("colorMod", yeCreateQuadInt(255, 100, 0, 255))
			    yGenericHandlerRefresh(pc_handler)
			}
			yeSetIntAt(pc_canel, PC_HURT, 7);
			yeSetIntAt(pc_canel, PC_DROPSPEED_IDX, -25);
			print_life(wid, pc, pc_canel)
			return true
		    }
		}
		if (ctype == TYPE_PIKE || ctype == TYPE_MONSTER || ctype == TYPE_BOSS)
		    return false;
		if ((ywPosY(old_pos) + SPRITE_SIZE) <= ywPosY(ywCanvasObjPos(c))) {
		    stop_fall = true
		    print_life(wid, pc, pc_canel)
		} else if (ctype != TYPE_LIGHT_FLOOR &&
			   (wid.geti("#-yblock") > 0 || yeGetIntAt(pc_canel, PC_DROPSPEED_IDX) >= 0) &&
			   (ywPosY(old_pos) + SPRITE_SIZE - 1) >
			   ywPosY(ywCanvasObjPos(c)) + 10) {
		    stop_x = true
		    if (wid.geti("#-yblock") > 0 && pc_canel.geti(PC_DROPSPEED_IDX) < 0) {
			pc_canel.setAt(PC_DROPSPEED_IDX, 0)
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
    let animations = wid.get("animations")
    if (animations) {
	for (a of animations) {
	    a.addAt(1, -turn_timer)
	    if (a.geti(1) < 0) {
		let ret = 2
		if (a.get(2)) {
		    ret = a.get(2).call(wid, a)
		}
		if (ret == 2) {
		    ywCanvasRemoveObj(wid, a.get(0))
		    animations.rm(a)
		}
	    }
	}
    }

}

function init_map(wid, map_str, pc_pos_orig)
{
    let pc = yeGet(wid, "pc")
    let pc_canel = yeGet(wid, "_pc")
    let pc_pos = yeCreateCopy(pc_pos_orig)
    yeReplaceBack(wid, ygFileToEnt(YJSON, map_str + ".json"), "_mi")
    let mi = yeGet(wid, "_mi")
    let parent_str = yeGet(mi, "parent")
    if (parent_str) {
	let parent = ygFileToEnt(YJSON, yeGetString(parent_str) + ".json");
	yeMergeInto(mi, parent, 0);
    }
    map_str = ygFileToEnt(YRAW_FILE, map_str);
    let map_str_a = yeGetString(map_str).split('\n');
    let map_a = yeReCreateArray(wid, "_m");
    let boss_i = yeGet(mi, "boss");
    let boss = null;
    let keydown = wid.setAt("keydown", 0)

    if (!boss_i) {
	yeRemoveChildByStr(wid, "_boss")
    } else {
	boss = yeReCreateArray(wid, "_boss")
    }

    if (pc_pos) {
	let pc_cam = yeGet(pc_canel, PC_POS_IDX)
	ywPosMultXY(pc_pos, SPRITE_SIZE, SPRITE_SIZE)
	yePrint(pc_pos)
	ywPosSet(pc_cam, pc_pos)
	yePrint(pc_cam)
	yeReplaceBack(wid, pc_cam, "cam")
	for (let l in map_str_a) {
	    yeCreateString(map_str_a[l], map_a)
	}
    } else {
	for (let l in map_str_a) {
	    yeCreateString(map_str_a[l], map_a)
	    let guy_index = map_str_a[l].indexOf('@')
	    if (guy_index > -1) {
		let pc_cam = yeGet(pc_canel, PC_POS_IDX)
		ywPosSetInts(pc_cam, guy_index * SPRITE_SIZE, l * SPRITE_SIZE)
		yePrint(pc_cam)
		yeReplaceBack(wid, pc_cam, "cam")
	    }
	}
    }
    let size = yeGet(mi, "size")
    let map_pixs_l = ywSizeCreate(ywSizeW(size) * SPRITE_SIZE, ywSizeH(size) * SPRITE_SIZE)
    yeReplaceBack(wid, map_pixs_l, "map-pixs-l")
    let monsters = yeReCreateArray(wid, "_monsters")

    map_a.forEach(function(s, i) {
	s = yeGetString(s)
	for (let j = 0; j < s.length; ++j) {
	    let c = s[j];
	    c_char_code = c.charCodeAt(0);
	    if (c_char_code >= "a".charCodeAt(0) && c_char_code <= "z".charCodeAt(0)) {
		let mon = yeCreateArray(monsters, c)
		yeCreateString(c, mon) // MONSTER_STR_KEY 0
		ywPosCreate(j * SPRITE_SIZE, i * SPRITE_SIZE, mon) // MONSTER_POS 1
		y_mover_new(mon) // MONSTER_MOVER 2
	    }
	}

    })

    yeSetIntAt(pc_canel, PC_NB_TURN_IDX, 0)

    print_all(wid)
    if (boss_i) {
	let bpos = yeGet(boss_i, "pos");

	print(" === ===")
	yePrint(boss_i)
	yePrint(bpos)
	print(" === ===----")

	let canvasobj = null
	let path = boss_i.get("path")

	if (yeType(path) == YSTRING) {
	    canvasobj = ywCanvasNewImgByPath(wid, ywPosX(bpos), ywPosY(bpos),
					     path)
	} else {
	    const size = path.get(1)
	    path = path.gets(0)

	    canvasobj = ywCanvasNewImg(wid, ywPosX(bpos), ywPosY(bpos),
				       path, size)
	}

	yeCreateIntAt(TYPE_BOSS, canvasobj, "amap-t", YCANVAS_UDATA_IDX)

	yePushBack(boss, canvasobj);
	y_mover_new(boss)
	let base_txt = "BOSS life: |"
	for (let life = yeGetIntAt(boss_i, "life"); life > 0; life -= 5) {
	    base_txt = base_txt + "="
	}
	base_txt = base_txt + "|"
	yePushBack(boss, ywCanvasNewTextByStr(wid, 0, 0, base_txt))
    }

    yeSetIntAt(pc_canel, PC_DROPSPEED_IDX, 0);
    y_move_set_xspeed(yeGet(pc_canel, PC_PUNCH_MINFO), 0)
    y_move_set_yspeed(yeGet(pc_canel, PC_PUNCH_MINFO), 0)
    y_move_set_xspeed(yeGet(pc_canel, PC_MOVER_IDX), 0)
    y_move_set_yspeed(yeGet(pc_canel, PC_MOVER_IDX), 0)

    let hooks = yeGet(mi, "hooks")
    if (hooks) {
	hooks.forEach(function (h, idx) {
	    if (!yeGet(h, "at")) {
		ywidActions(wid, h)
		yeRemoveChildByIdx(hooks, idx)
	    }
	})
    }
}

function amap_init(wid)
{
    //yePrint(wid)
    ywSetTurnLengthOverwrite(-1)
    yeCreateFunction(amap_action, wid, "action")

    let pc = yeGet(wid, "pc")
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
	let stats = yeCreateArray(pc, "stats");
	yeCreateInt(6, stats, "agility");
	yeCreateInt(4, stats, "strength");
	// pc.setAt("jmp-power", 10)
    }
    //yePrint(pc)
    // canel for canvas element, it's the info about the screen position and stuff
    let pc_canel = yeCreateArray(wid, "_pc")

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

    let txts_arrays = yeCreateArray()
    yePrint(wid.get("pc-sprites"))
    if (wid.get("pc-sprites") != null) {
	ygModDirOut()
	for (s of wid.get("pc-sprites")) {
	    ywTextureNewImg(yeGetString(s), null, txts_arrays, null)
	}
	ygModDir("amap")
    } else {
	yePushBack(txts_arrays, ywTextureNewImg("./gut-0.png", null, textures, "guy-0"));
	yePushBack(txts_arrays, ywTextureNewImg("./gut-1.png", null, textures, "guy-1"));
    }

    let pc_handler = yGenericNewTexturesArray(wid, txts_arrays,
					      yeCreateArray(), ywPosCreate(0, 0),
					      wid, "pc_handler");

    ygModDirOut()
    if (wid.get("pc-jmp-sprites")) {
	let jmp_array = yeCreateArray()
	for (s of wid.get("pc-jmp-sprites")) {
	    ywTextureNewImg(yeGetString(s), null, jmp_array, null);
	}
	pc_handler.get("txts").push(jmp_array, "jmp")
    }

    if (wid.get("pc-dash-sprites")) {
	let dash_array = yeCreateArray()
	for (s of wid.get("pc-dash-sprites")) {
	    ywTextureNewImg(yeGetString(s), null, dash_array, null);
	}
	pc_handler.get("txts").push(dash_array, "dash")
    }

    if (wid.get("pc-punch-sprites")) {
	let punch_array = yeCreateArray()
	for (s of wid.get("pc-punch-sprites")) {
	    ywTextureNewImg(yeGetString(s), null, punch_array, null);
	}
	pc_handler.get("txts").push(punch_array, "punch")
    }
    if (wid.get("walk-sound")) {
	let sound_path = wid.get("walk-sound").s()

	let sound = ySoundLoad(sound_path)
	print("sound: ", sound)
	wid.setAt("_walk_sound", sound)
	ySoundLevel(sound, 40)
    }

    if (wid.get("atk-sound")) {
	let sound_path = wid.get("atk-sound").s()

	let sound = ySoundLoad(sound_path)
	print("sound: ", sound)
	wid.setAt("_atk_sound", sound)
	ySoundLevel(sound, 50)
    }

    ygModDir("amap")

    if (wid.get("attack-sprite")) {
	ygModDirOut();
	ywTextureNewImg(wid.gets("attack-sprite"), null, textures, "punch");
	ygModDir("amap")
    } else {
	ywTextureNewImg("./punch.png", null, textures, "punch");
    }
    ywTextureNewImg("./motivation.png", null, textures, "motivation");
    ywTextureNewImg("./uwu-head.png", null, textures, "uwu-head");
    ywTextureNewImg("./gamu.png", null, textures, "gamu");
    ywTextureNewImg("./pillow.png", null, textures, "pillow");
    ywTextureNewImg("./vodeo-monster.png", null, textures, "vodeo-monster");
    ygModDirOut();

    let extra_textures = wid.get("extra-textures")
    if (extra_textures) {
	for (let i = 0; i < yeLen(extra_textures); ++i) {
	    let name = yeGetKeyAt(extra_textures, i)
	    let txt = extra_textures.get(i)
	    let rect = null

	    if (yeType(txt) != YSTRING) {
		rect = txt.get(1)
		txt = txt.get(0)
 	    }
	    ywTextureNewImg(txt.s(), rect, textures, name);
	}
    }

    yeCreateIntAt(0, pc_canel, "jmp-n", PC_JMP_NUMBER)
    yeCreateIntAt(0, pc_canel, "hurt", PC_HURT)
    yeCreateArrayAt(pc_canel, "life-array", PC_LIFE_ARRAY)
    yeCreateIntAt(0, pc_canel, "pl", PC_PUNCH_LIFE)
    yeCreateIntAt(0, pc_canel, "dash", PC_DASH)
    yeCreateIntAt(DIR_RIGHT, pc_canel, "dir", PC_DIR)
    yeCreateIntAt(0, pc_canel, "nb_turn", PC_NB_TURN_IDX)
    y_mover_new_at(pc_canel, "p_minfo", PC_PUNCH_MINFO)
    init_map(wid, map)
    return ret
}

function win(wid)
{
    print("YOU WIN !!!");
    ygCallFuncOrQuit(wid, "win");
    return 1
}

function next(wid, _, pos)
{
    let mi = yeGet(wid, "_mi")
    init_map(wid, yeGetStringAt(mi, "next"), pos)
    return 1
}

function next1(wid, _, pos)
{
    let mi = yeGet(wid, "_mi")
    init_map(wid, yeGetStringAt(mi, "next1"), pos)
    return 1
}

function next2(wid, _, pos)
{
    let mi = yeGet(wid, "_mi")
    init_map(wid, yeGetStringAt(mi, "next2"), pos)
    return 1
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
}

function mod_init(mod)
{
    ygInitWidgetModule(mod, "amap", yeCreateFunction("amap_init"))
    yeCreateString("lvl-test", yeGet(mod, "test_wid"), "map")
    yeCreateString("rgba: 255 255 255 255", yeGet(mod, "test_wid"), "background")
    yeCreateFunction(next, mod, "next")
    yeCreateFunction(next1, mod, "next1")
    yeCreateFunction(next2, mod, "next2")
    yeCreateFunction("win", mod, "win")
    let mons_mv = yeCreateArray(mod, "mons_mv")
    yeCreateFunction(monster_left_right, mons_mv, "left_right")
    yeCreateFunction(monster_rand, mons_mv, "rand")
    yeCreateFunction(monster_round, mons_mv, "round")
    ygRegistreFunc(3, "yamap_push_obj", "yamap_push_obj")
    ygRegistreFunc(5, "yamap_generate_monster_canvasobj",
		   "yamap_generate_monster_canvasobj")
    ygAddModule(Y_MOD_YIRL, mod, "smart_cobject")
    ygAddModule(Y_MOD_YIRL, mod, "stop-screen")
    ygAddModule(Y_MOD_YIRL, mod, "y_move")
    return mod
}
