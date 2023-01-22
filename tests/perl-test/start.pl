#!/usr/bin/perl

sub mod_init
{
    print "in mod_init !!!\n";
    $mod = $_[0];
    $grp = Yirl::yevCreateGrp(0, 1,2,3);
    Yirl::yePrint($mod);
    Yirl::yePrint($grp);
    return $mod;
}

print "Hello world\n";

