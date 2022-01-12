class Main inherits IO
{
    puts(s : String) : SELF_TYPE {
        out_string(s.concat("\n"))
    };

    main(): Int {{
        puts("www"); -- www
        abort();
        puts("yyy");
        puts("nnn");
        0;
    }};
};
