class Main inherits IO {
    main(): Int {{
        let i : Int in {
            while i < 11 loop {
                out_string(fib(i).to_string().concat("\n"));
                i <- i + 1;
            } pool;
        };
        0;
    }};

    fib(n : Int) : Int {
        if n <= 0 then
            0
        else
            if n = 1 then
                1
            else
                fib((n-1)) + fib((n-2))
            fi
        fi
    };
};

-- 0
-- 1
-- 1
-- 2
-- 3
-- 5
-- 8
-- 13
-- 21
-- 34
-- 55
