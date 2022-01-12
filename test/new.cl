class Foo
{
    s : String;

    set_s(x : String) : SELF_TYPE {{
        s <- x;
        self;
    }};

    get_s() : String {
        s
    };
};

class Main inherits IO
{
    f1 : Foo <- new Foo;

    main(): Int {{
        f1.set_s("brown");
        puts(f1.get_s()); -- brown

        let f2 : Foo <- new Foo in {
            f2.set_s("fox");
            puts(f2.get_s()); -- fox
        };

        0;
    }};

    puts(s : String) : SELF_TYPE {
        out_string(s.concat("\n"))
    };
};
