

sub same {
    return $_[0];
}

my $atk = 1;

$atk = same(-$atk);

print $atk, "\n";

$s = "same: " . same(13) . "\n";

print($s);

print(same(13), "\n");
