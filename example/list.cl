class ListNode {
    prev : ListNode;
    next : ListNode;
    data : Object;

    get_prev() : ListNode {
        prev
    };

    set_prev(node : ListNode) : SELF_TYPE {{
        prev <- node;
        self;
    }};

    get_next() : ListNode {
        next
    };

    set_next(node : ListNode) : SELF_TYPE {{
        next <- node;
        self;
    }};

    get_data() : Object {
        data
    };

    set_data(obj : Object) : SELF_TYPE {{
        data <- obj;
        self;
    }};
};

class List {
    head : ListNode <- new ListNode;
    tail : ListNode <- new ListNode;

    init() : SELF_TYPE {{
        head.set_next(tail);
        tail.set_prev(head);
        self;
    }};

    append(obj : Object) : SELF_TYPE {{
        let node : ListNode <- new ListNode in {
            node.set_data(obj);
            node.set_prev(tail.get_prev());
            node.set_next(tail);
            tail.get_prev().set_next(node);
            tail.set_prev(node);
        };
        self;
    }};

    prepend(obj : Object) : SELF_TYPE {{
        let node : ListNode <- new ListNode in {
            node.set_data(obj);
            node.set_prev(head);
            node.set_next(head.get_next());
            head.get_next().set_prev(node);
            head.set_next(node);
        };
        self;
    }};

    get_head() : ListNode {
        head
    };

    get_tail() : ListNode {
        tail
    };
};

class Main inherits IO {
    main(): Int {{
        let list : List <- new List in {
            list.init();

            let i : Int <- 1 in
                while i <= 10 loop {
                    list.append(i);
                    i <- i + 1;
                } pool
            ;

            let node : ListNode <- list.get_head().get_next() in
                while not isvoid node.get_next() loop {
                    case node.get_data() of
                        i : Int => out_string(i.to_string().concat("\n"));
                    esac;
                    node <- node.get_next();
                } pool
            ;
        };
        0;
    }};
};
