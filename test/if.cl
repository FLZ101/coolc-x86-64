class Main inherits IO {
    b1 : Bool <- 100 < 200;
    b2 : Bool <- 100 = 200;
    b3 : Bool <- true;
    b4 : Bool <- false;
    b5 : Bool <- 100 <= 100;
    b6 : Bool <- 200 <= 100;
    b7 : Bool <- not false;
    b8 : Bool <- not 100 <= 100;

    s1 : String <- "Y";
    s2 : String <- "N";

    main(): Int
    {{
        putb(b1); -- Y
        putb(b2); -- N
        putb(b3); -- Y
        putb(b4); -- N
        putb(b5); -- Y
        putb(b6); -- N
        putb(b7); -- Y
        putb(b8); -- N
        0;
    }};

    putb(b : Bool) : SELF_TYPE {
        out_string((if b then s1 else s2 fi).concat("\n"))
    };
};
