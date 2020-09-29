### Compilation failed:

Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify ST float x = 0.0 
simplifying vardecl
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify ST float x = 0.0; 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify ST switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
} 
simplifying switch
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX (x = 0.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify ST (x = 0.0); 
simplifying expr
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (x < sqrt(1.0))
Node 9 (0x613000001b58): if ((x < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX x 
optimized to 0.0 
coerced to 0.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX sqrt(1.0) 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX (0.0 < sqrt(1.0)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify ST if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}
 
simplifying if
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(x)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX x 
optimized to 0.0 
coerced to 0.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX half(0.0) 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX half4(half(0.0)) 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): flo=================================================================
==77854==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000019e50 at pc 0x000101589c05 bp 0x7ffeee78bdf0 sp 0x7ffeee78bde8
READ of size 8 at 0x602000019e50 thread T0
    #0 0x101589c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10158abad in SkSL::BasicBlock::Node::description() const SkSLCFGGenerator.h:65
    #2 0x101589069 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:79
    #3 0x101587fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #4 0x10162dd91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #5 0x10163c657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #6 0x1016396e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #7 0x101475108 in main SkSLMain.cpp:258
    #8 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x602000019e50 is located 0 bytes inside of 16-byte region [0x602000019e50,0x602000019e60)
freed by thread T0 here:
    #0 0x1031a6c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x101491f9c in std::__1::_DeallocateCaller::__do_call(void*) new:334
    #2 0x101491f40 in std::__1::_DeallocateCaller::__do_deallocate_handle_size(void*, unsigned long) new:292
    #3 0x10188645c in std::__1::_DeallocateCaller::__do_deallocate_handle_size_align(void*, unsigned long, unsigned long) new:268
    #4 0x1018863ec in std::__1::__libcpp_deallocate(void*, unsigned long, unsigned long) new:340
    #5 0x1018df8f1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::deallocate(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1872
    #6 0x1018df4dc in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::deallocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1594
    #7 0x1018deeab in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:464
    #8 0x101c3f29d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #9 0x101c39371 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #10 0x101c392a0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #11 0x1016694b4 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #12 0x10188bda4 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #13 0x101889eb1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #14 0x101889f11 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #15 0x1016d41f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #16 0x1016d3bdd in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #17 0x10162015d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #18 0x101621e24 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719
    #19 0x10161e4b7 in SkSL::Compiler::simplifyExpression(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:925
    #20 0x10162deff in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1519
    #21 0x10163c657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #22 0x1016396e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #23 0x101475108 in main SkSLMain.cpp:258
    #24 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x1031a684d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x101884e64 in std::__1::__libcpp_allocate(unsigned long, unsigned long) new:253
    #2 0x1018e34b1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::allocate(unsigned long, void const*) memory:1869
    #3 0x1018e2fa8 in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::allocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, unsigned long) memory:1586
    #4 0x1018e2c0c in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:318
    #5 0x1018e1d2b in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:317
    #6 0x101863341 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::reserve(unsigned long) vector:1586
    #7 0x101888af4 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:55
    #8 0x1017f54e7 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:54
    #9 0x1018459eb in std::__1::__unique_if<SkSL::BinaryExpression>::__unique_single std::__1::make_unique<SkSL::BinaryExpression, int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*&>(int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Type const*&) memory:3033
    #10 0x10181a7b0 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1962
    #11 0x1017e6c39 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #12 0x1017cde58 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #13 0x1017c157b in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #14 0x1017d20e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #15 0x1017ca2a0 in SkSL::IRGenerator::convertSwitch(SkSL::ASTNode const&) SkSLIRGenerator.cpp:682
    #16 0x1017c1377 in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:232
    #17 0x1017d20e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #18 0x1017c2a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #19 0x1018058cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #20 0x10186a663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #21 0x101638e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #22 0x101475108 in main SkSLMain.cpp:258
    #23 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c0400003370: fa fa fd fa fa fa fd fd fa fa fd fa fa fa 00 00
  0x1c0400003380: fa fa fd fa fa fa fd fd fa fa fd fd fa fa fd fd
  0x1c0400003390: fa fa 00 fa fa fa fd fa fa fa fd fa fa fa fd fa
  0x1c04000033a0: fa fa fd fa fa fa 00 00 fa fa 00 fa fa fa 00 fa
  0x1c04000033b0: fa fa 00 00 fa fa fd fa fa fa 00 00 fa fa 00 00
=>0x1c04000033c0: fa fa fd fa fa fa fd fa fa fa[fd]fd fa fa 00 fa
  0x1c04000033d0: fa fa 00 00 fa fa 00 00 fa fa 00 fa fa fa fd fa
  0x1c04000033e0: fa fa 00 00 fa fa fd fd fa fa 00 00 fa fa 00 00
  0x1c04000033f0: fa fa 00 00 fa fa fd fd fa fa 00 00 fa fa fd fd
  0x1c0400003400: fa fa 00 00 fa fa 00 00 fa fa fd fd fa fa fd fd
  0x1c0400003410: fa fa fd fd fa fa fd fd fa fa fd fd fa fa fd fd
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
  Shadow gap:              cc
==77854==ABORTING
at x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX (sk_FragColor = half4(half(0.0))) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify ST (sk_FragColor = half4(half(0.0))); 
simplifying expr
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify ST break; 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
(x = 1.0);
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): x
Node 2 (0x610000001070): 1.0
Node 3 (0x610000001088): (x = 1.0)
Node 4 (0x6100000010a0): (x = 1.0);
Exits: [1]

Block 7
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify EX (x = 1.0) 
optimizing binary 
deadass? update=1 rescan=0 
deadass! update=1 rescan=0 
delete_left update=1 rescan=0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 0.0
Node 1 (0x610000000d58): float x = 0.0
Node 2 (0x610000000d70): float x = 0.0;
Node 3 (0x610000000d88): 0
Node 4 (0x610000000da0): switch (0) {
case 0:
(x = 0.0);
if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

case 1:
1.0;
}
Exits: [1, 2, 6]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3, 6]
Exits: [7]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): x
Node 2 (0x613000001ab0): 0.0
Node 3 (0x613000001ac8): (x = 0.0)
Node 4 (0x613000001ae0): (x = 0.0);
Node 5 (0x613000001af8): 0.0
Node 6 (0x613000001b10): 1.0
Node 7 (0x613000001b28): sqrt(1.0)
Node 8 (0x613000001b40): (0.0 < sqrt(1.0))
Node 9 (0x613000001b58): if ((0.0 < sqrt(1.0))) {
(sk_FragColor = half4(half(0.0)));
break;
}

Exits: [3, 5]

Block 3
-------
Before: [float x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half(0.0)
Node 3 (0x610000000f88): half4(half(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(0.0)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(0.0)));
Node 6 (0x610000000fd0): break;
Exits: [1]

Block 4
-------
Before: []
Entrances: []
Exits: []

Block 5
-------
Before: [float x = 0.0]
Entrances: [2]
Exits: [6]

Block 6
-------
Before: [float x = <defined>]
Entrances: [0, 5]
Node 0 (0x610000001040): 1
