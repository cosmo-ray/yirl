#!/usr/bin/perl

sub mod_init
{
    print "in mod_init !!!\n";
    $mod = $_[0];
    Yirl::yePrint($mod);
    $grp = Yirl::yevCreateGrp(0, 1,2,3);
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
    print "------------\n";
    return $mod;
}

print "Hello world\n";

