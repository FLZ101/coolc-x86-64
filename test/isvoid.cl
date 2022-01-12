class Foo {};

class Main inherits IO
{
    i1 : Int;
    i2 : Int <- 100;
    s1 : String;
    s2 : String <- "brown fox";
    b1 : Bool;
    b2 : Bool <- true;
    f1 : Foo;
    f2 : Foo <- new Foo;

    main(): Int
    {{
        putb(isvoid i1); -- N
        putb(isvoid i2); -- N
        putb(isvoid s1); -- N
        putb(isvoid s2); -- N
        putb(isvoid b1); -- N
        putb(isvoid b2); -- N
        putb(isvoid f1); -- Y
        putb(isvoid f2); -- N
        putb(isvoid self); -- N
        0;
    }};

    putb(b : Bool) : SELF_TYPE {
        out_string((if b then "Y\n" else "N\n" fi))
    };
};
