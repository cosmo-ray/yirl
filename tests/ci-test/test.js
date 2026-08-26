function test(e) {
    print("ci: start");
    var t = yeCreateArray(null);
    yeCreateInt(42, t, "val");
    yeCreateString("works", t, "msg");
    print("ci: val=" + yeGetIntAt(t, "val"));
    print("ci: msg=" + yeGetStringAt(t, "msg"));
    yeDestroy(t);
    print("ci: done");
    return null;
}
