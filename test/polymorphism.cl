class A inherits IO
{
    say() : SELF_TYPE {
        puts("fox")
    };

    puts(s : String) : SELF_TYPE {
        out_string(s.concat("\n"))
    };
};

class B inherits A {};

class C inherits A {
    say() : SELF_TYPE {
        puts("dog")
    };
};

class D inherits C {
    puts(s : String) : SELF_TYPE {
        out_string(s.concat(" *\n"))
    };
};

class Main
{
    main(): Int {{
        let a1 : A <- new A,
            a2 : B <- new B,
            a3 : C <- new C,
            a4 : D <- new D
        in {
            a1.say(); -- fox
            a2.say(); -- fox
            a3.say(); -- dog
            a4.say(); -- dog *

            a1@A.say(); -- fox
            a2@A.say(); -- fox
            a3@A.say(); -- fox
            a4@A.say(); -- fox *
        };
        0;
    }};
};
