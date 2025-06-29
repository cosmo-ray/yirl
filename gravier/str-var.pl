$in_str = "moi";
$in_str2 = "toi";
$str="hello $in_str\n";
print $str;
$str="$in_str et $in_str2\n";
print $str;

$s = "Hello #" =~ s/#/$str/r;
print $s;
