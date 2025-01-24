
my $oui = "oui";

sub oui
{
    if ($oui eq "skip") {
	return;
    }
    print $oui;
}

oui;
$oui = "non";
oui();
print("\n");
$oui = "skip";
oui;

sub print_01 {
    print $_[0];
}

sub print_01_nl
{
    print_01 $_[0];
    print_01("\n");
}

sub print_02_nl
{
    sub print_02 {
	print $_[1];
    }
    print_02 $_[0]; # nothing will be print here
    print_02("\n", "humm\n");
}

print_01 "oh";
print_01_nl "ah";

print_02 "oh";
print_02_nl "ah";

print_01 3 + 4;
print_01 "\n";
