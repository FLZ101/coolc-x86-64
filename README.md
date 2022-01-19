A [COOL](https://web.stanford.edu/class/cs143/) compiler for x86-64.

## Build

```
$ make
```

## Test

```
$ make test
```

## Examples

### hello

```
-- example/hello.cl

class Main inherits IO {
    main(): Int {{
        out_string("hello world\n");
        0;
    }};
};
```

```
$ src/coolc example/hello example/hello.cl
$ example/hello
hello world
```

### recursion

```
-- test/recursion.cl

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
                fib(n-1) + fib(n-2)
            fi
        fi
    };
};
```

```
$ src/coolc test/recursion test/recursion.cl
$ test/recursion
0
1
1
2
3
5
8
13
21
34
55
```

