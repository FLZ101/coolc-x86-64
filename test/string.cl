class Main inherits IO
{
    main(): Int
    {{
        let s1 : String <- "12345",
            s2 : String <- s1.substr(1, 4),
            i1 : Int <- s1.to_int() + 1,
            i2 : Int <- s2.to_int() + 1
        in {
            puts(s1); -- 12345
            puts(s2); -- 234
            puts(i1.to_string()); -- 12346
            puts(i2.to_string()); -- 235
        };
        0;
    }};

    puts(s: String) : SELF_TYPE
    {{
        out_string(s.concat("\n"));
        self;
    }};
};
