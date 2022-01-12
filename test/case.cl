class A inherits IO
{
    say() : SELF_TYPE {
        puts(type_name())
    };

    puts(s : String) : SELF_TYPE {
        out_string(s.concat("\n"))
    };
};

class B inherits A {
    sayB() : SELF_TYPE {
        puts("  www")
    };
};

class C inherits A {
    sayC() : SELF_TYPE {
        puts("  yyy")
    };
};

class D inherits C {
    sayD() : SELF_TYPE {
        puts("  nnn")
    };
};

class Main
{
    main(): Int {{
        let a1 : A <- new A,
            a2 : A <- new B,
            a3 : A <- new C,
            a4 : A <- new D
        in {
            say(a1); -- A
            say(a2); -- B
                     --   www
            say(a3); -- C
                     --   yyy
            say(a4); -- D
                     --   nnn
        };
        0;
    }};

    say(a : A) : SELF_TYPE {{
        a.say();
        case a of
            a : A => a;
            b : B => b.sayB();
            c : C => c.sayC();
            d : D => d.sayD();
        esac;
        self;
    }};
};
