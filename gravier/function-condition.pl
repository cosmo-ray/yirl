
sub one {
    return 1;
}

sub zero {
    return 0;
}

sub this_num {
    return $_[0];
}

print this_num(one()), "\n";

if (one() > zero()) {
    print "one is superior to zero\n";
} else {
    print("BUG");
}

if (one() < zero()) {
    print("BUG");
} else {
    print "one is still superior to zero\n";
}


#if (one() > zero()) {
#    print "one is superior to zero\n";
#}

