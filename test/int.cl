class Main inherits IO
{
    main(): Int
    {{
        let i1 : Int <- 1,
            i2 : Int <- 2 + 3,
            i3 : Int <- i1 * 4,
            i4 : Int <- (i1 + i3) * i2 - i3,
            i5 : Int <- (i4 - i1) / 6,
            i6 : Int <- (i5 - i4) / i5,
            i7 : Int <- ~i6 + 1
        in {
            puti(i1); -- 1
            puti(i2); -- 5
            puti(i3); -- 4
            puti(i4); -- 21
            puti(i5); -- 3
            puti(i6); -- -6
            puti(i7); -- 7
        };
        0;
    }};

    puti(i: Int) : SELF_TYPE
    {{
        out_string(i.to_string().concat("\n"));
        self;
    }};
};
