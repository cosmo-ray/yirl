

sub same {
    return $_[0];
}

my $atk = 1;

$atk = same(-$atk);

print $atk, "\n";
