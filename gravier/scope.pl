#!/usr/bin/perl

{
    $a = "a";
    my $b = "b";
    $c = "c";
    my $d = "d";
    {
	my $b = "B";
	print($b, "\n");
    }
    print $a,$b,$c,$d,"\n";
}

print $a,$b,$c,$d,"\n";
