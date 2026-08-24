<?php
print("reading php file !!!!\n");

function display_eint($e)
{
    print(yeGetInt($e));
    print("\n");
    return yeGetInt($e);
}

function mod_init($e)
{
    print("ph7 mod_init !!\n");
    return yirl_return(0);
}

function mk_hello($e, $name)
{
    return yeCreateString("hello world !", $e, $name);
}

function mk_hello2()
{
    return yeCreateString("hello world !");
}

function make_storage($father)
{
    $st = yeCreateArray($father);
    yeCreateInt(0, $st);
    return $st;
}

function inc_storage($st)
{
    yeIncrAt($st, 0);
}

function read_storage($st)
{
    return yeGetIntAt($st, 0);
}

function accumulate($i)
{
    static $total = 0;
    $total += $i;
    return $total;
}
?>
