
sub this {
    return $_[0];
}

sub conc {
    return $_[0] . $_[1];
}

print this(this("a\n"));

$r = conc("a", "b\n");
print $r;

$r = conc this(this("c")), this("d\n");

print $r;
