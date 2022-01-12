class Main inherits IO {
    main(): Int {{
        let i : Int <- 1, total : Int
        in {
            while i <= 100 loop {
                total <- total + i;
                i <- i + 1;
            } pool;
            out_string(total.to_string().concat("\n")); -- 5050
        };
        0;
    }};
};
