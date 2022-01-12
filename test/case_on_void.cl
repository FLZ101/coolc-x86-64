class A inherits IO
{
    sayA() : SELF_TYPE {
        puts("A")
    };

    puts(s : String) : SELF_TYPE {
        out_string(s.concat("\n"))
    };
};

class B inherits A {
    sayB() : SELF_TYPE {
        puts("B")
    };
};

class C inherits A {
    sayC() : SELF_TYPE {
        puts("C")
    };
};

class Main
{
    main(): Int {{
        let a : A
        in {
            say(a); -- fatal error: case on void
        };
        0;
    }};

    say(a : A) : SELF_TYPE {{
        case a of
            a : A => a.sayA();
            b : B => b.sayB();
        esac;
        self;
    }};
};
