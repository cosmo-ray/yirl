
print("reading file !!!!")

function display_ent(e)
{
    print("display ent !!!!")
    return 3;
}

function display_eint(e)
{
    print(yeGetInt(e))
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
