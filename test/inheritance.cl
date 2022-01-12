class A inherits IO
{
    i1 : Int <- {
        puts("A");
        100;
    };

    s3 : String <- "333";

    say() : SELF_TYPE {
        puts(s3)
    };

    puts(s : String) : SELF_TYPE {
        out_string(s.concat("\n"))
    };
};

class B inherits A {
    i2 : Int <- {
        puts("B");
        200;
    };

    s2 : String <- "222";
    s4 : String <- "444";
};

class C inherits B {
    i3 : Int <- {
        puts("C");
        300;
    };

    s5 : String <- "555";
    s1 : String <- "111";
};

class Main
{
    main(): Int {{
        let a : A <- new A, -- A
            b : B <- new B, -- A
                            -- B
            c : C <- new C  -- A
                            -- B
                            -- C
        in {
            a.say(); -- 333
            b.say(); -- 333
            c.say(); -- 333
        };
        0;
    }};
};
