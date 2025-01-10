#!/usr/bin/perl

my $i = 0;

for ($i = 0; $i <= 100; $i += 5) {
    print $i, " \n";
}


while ($j < 10) {
    print $j, "\n";
    ++$j
}

for (; $j > 0; --$j)
{
}

print "now:", $i, " - ", $j, "\n";
