class Main inherits IO
{
    a : Int <- 100;
    b : Int <- 200;

    main(): Int
    {{
        puti(a); -- 100
        puti(b); -- 200

        let b : Int <- 300,
            c : Int <- 400
        in {
            puti(a); -- 100
            puti(b); -- 300
            puti(c); -- 400

            let a : Int <- 500,
                c : Int <- 600
            in {
                puti(a); -- 500
                puti(b); -- 300
                puti(c); -- 600
            };

            puti(a); -- 100
            puti(b); -- 300
            puti(c); -- 400
        };

        puti(a); -- 100
        puti(b); -- 200

        0;
    }};

    puti(i: Int) : SELF_TYPE
    {{
        out_string(i.to_string().concat("\n"));
        self;
    }};
};
