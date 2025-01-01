#!/usr/bin/perl

$nb = 0;

if ($nb == 0) {
    print("nb is 0\n");
    $nb = 1;
    print("nb: ", $nb, "\n");
}

print("nb again: ", $nb, "\n");

if ($nb == 0) {
    print("nb is still 0\n");
} else {
    print "nb is not 0 annymore\n";
}

$nb = 1;

if ($nb == 0) {
    print("nb is still 0 AGAIN\n");
} elsif ($nb == 1) {
    print "nb is 1 again and again\n";
} elsif ($nb == 2) {
    print "and now it's 2\n";
} else {
    print "wtf\n";
}
