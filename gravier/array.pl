
$ar2[1] = "tata";
$ar2[0] = "titi";

print $ar2[0], "\n";
print $ar2[1], "\n";

@ar = (25, 30, "out");

print $ar[0], "\n";
print $ar[1], "\n";
print $ar[2], "\n";

print $ar[$none], "\n";
$ar[$none] = 7;
print $ar[$none], "\n";

$one=1;
print $ar[$one], "\n";

$out = $ar[2];

print $out, "\n";

$add = $ar[1] + $ar[0];

print $add, "\n";
