class Foo {};

class Main inherits IO
{
    puts(s : String) : SELF_TYPE {
        out_string(s.concat("\n"))
    };

    main(): Int {
        let f : Foo
        in {
            puts(f.type_name()); -- fatal error: invoke on void
            puts("wyn");
            0;
        }
    };
};
