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

function mod_init(mod)
{
    ygRegistreFunc(2, "y_mover_new", "y_mover_new");
    ygRegistreFunc(3, "y_mover_new_at", "y_mover_new_at");

    ygRegistreFunc(3, "y_move_obj", "y_move_obj");
    ygRegistreFunc(3, "y_move_pos", "y_move_pos");

    ygRegistreFunc(1, "y_mover_init", "y_mover_init")
    ygRegistreFunc(1, "y_move_last_y", "y_move_last_y")
    ygRegistreFunc(1, "y_move_last_x", "y_move_last_x")
    ygRegistreFunc(1, "y_move_y_speed", "y_move_y_speed")
    ygRegistreFunc(1, "y_move_x_speed", "y_move_x_speed")

    ygRegistreFunc(2, "y_move_undo", "y_move_undo")
    ygRegistreFunc(2, "y_move_undo_x", "y_move_undo_x")
    ygRegistreFunc(2, "y_move_undo_y", "y_move_undo_y")


    ygRegistreFunc(2, "y_move_add_x_speed", "y_move_add_x_speed")
    ygRegistreFunc(2, "y_move_add_y_speed", "y_move_add_y_speed")

    ygRegistreFunc(2, "y_move_set_xspeed", "y_move_set_xspeed")
    ygRegistreFunc(2, "y_move_set_yspeed", "y_move_set_yspeed")
    return mod
}

