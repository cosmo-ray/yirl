
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
    #sub print_01 {
    #print $_[1];
    #}
    print_01 $_[0];
    print_01("\n");
}

print_01 "oh";

print_01_nl "ah";

