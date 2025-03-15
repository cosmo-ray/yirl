$eoc = <<EOC;
ho ho ho
a multy line
STR !!
EOC

print $eoc;

@ar = split "\n", $eoc;

print "------\n", uc($ar[1]), "\n------\n";

