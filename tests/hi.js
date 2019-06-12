
function display_ent(e)
{
    return true;
}

function display_eint(e)
{
    return yeGetInt(e);
}

function mk_hello(e, name)
{
    name = to_str(name)
    return yeCreateString("hello world !", e, name)
}

function mk_hello2(e, name)
{
    return yeCreateString("hello world !")
}
