

sub two {
    print("in two\n");
    return 2;
}

$r = two;

print $r, "\n";

sub ten {
    $ten = 10;
    return $ten;
}

$r = ten;

print $r, "\n";

sub this_str {
    $ts = "this str\n";
    return $ts;
}

$rs = this_str;

print $rs;

print this_str;

$add = two() + ten();

print $add, "\n";

sub add_this
{
    $ret = $_[0] + $_[1];
    return $ret;
}

$add = add_this(4, 7) + ten();

print $add, "\n";
