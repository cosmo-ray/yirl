
sub conc {
    print $_[0];
    return 3;
}

$r = conc("ah", conc("c"));

print "\n";
