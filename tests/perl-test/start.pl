#!/usr/bin/perl

sub widget_init
{
    $wid = $_[0];
    print "widget_init !!!!!!\n";
    print "widget_init !!!!!!\n";
    print "widget_init !!!!!!\n";
    print "widget_init !!!!!!\n";
    print "widget_init !!!!!!\n";
    return $wid
}

sub mod_init
{
    print "in mod_init !!!\n";
    $mod = $_[0];
    Yirl::yePrint($mod);
    $grp = Yirl::yevCreateGrp(0, 0,1,2,3);
    print "============\n";
    print $grp, "\n";
    Yirl::yePrint($grp);
    print "============\n";
    $re = Yirl::yeGetRandomElem($grp);
    print Yirl::yeGetInt($re), "\n";
    Yirl::yePrint($re);
    print Yirl::yeGetInt(Yirl::yeGetRandomElem($grp)), "\n";
    print Yirl::yeGetInt(Yirl::yeGetRandomElem($grp)), "\n";
    print Yirl::yeGetInt(Yirl::yeGetRandomElem($grp)), "\n";
    print Yirl::yeGetInt(Yirl::yeGetRandomElem($grp)), "\n";
    print Yirl::yuiAbs(-30), "\n";
    Yirl::ygReCreateString("ah.str", "helLO");
    Yirl::ygSetInt("ah.test", 123);
    Yirl::yePrint(Yirl::ygGet("ah.test"));
    Yirl::yePrint(Yirl::ygGet("ah.str"));
    print "------------\n";
    $callback = Yirl::yeCreateFunction("widget_init");
    Yirl::ygInitWidgetModule($mod, "perl-test !", widget_init);

    return $mod;
}

print "Hello world\n";

