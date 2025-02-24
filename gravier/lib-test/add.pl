sub test()
{
    $res = ATest::add(2, 4);
    print $res, "\n";
}

test;

$res = ATest::add(10, -7);
print $res, "\n";

$res = ATest::sub(10, -7);
print $res, "\n";

$res = "add: " . ATest::add(10, -7) . "\n";

print($res)
