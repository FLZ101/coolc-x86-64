class Main inherits IO {
    main(): Int
    {{
        let s : String <- in_string()
        in {
            out_string(s);
        };
        0;
    }};
};

-- the quick brown fox jumps over the lazy dog
