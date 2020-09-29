### Compilation failed:

Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010420): int x
Node 1 (0x608000010438): int x;
Node 2 (0x608000010450): 3
Node 3 (0x608000010468): switch (3) {
case 0:
(x = 0);
case 1:
(x = 1);
}
Exits: [1, 2, 3]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0, 3]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Exits: [4]

Block 2
-------
Before: [int x = <undefined>]
Entrances: [0]
Node 0 (0x610000000d40): 0
Node 1 (0x610000000d58): x
Node 2 (0x610000000d70): 0
Node 3 (0x610000000d88): (x = 0)
Node 4 (0x610000000da0): (x = 0);
Exits: [3]

Block 3
-------
Before: [int x = <undefined>]
Entrances: [0, 2]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): x
Node 2 (0x610000000e70): 1
Node 3 (0x610000000e88): (x = 1)
Node 4 (0x610000000ea0): (x = 1);
Exits: [1]

Block 4
-------
Before: [int x = <undefined>]
Entrances: [1]
Exits: []

about to simplify ST int x 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010420): int x
Node 1 (0x608000010438): int x;
Node 2 (0x608000010450): 3
Node 3 (0x608000010468): switch (3) {
case 0:
(x = 0);
case 1:
(x = 1);
}
Exits: [1, 2, 3]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0, 3]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Exits: [4]

Block 2
-------
Before: [int x = <undefined>]
Entrances: [0]
Node 0 (0x610000000d40): 0
Node 1 (0x610000000d58): x
Node 2 (0x610000000d70): 0
Node 3 (0x610000000d88): (x = 0)
Node 4 (0x610000000da0): (x = 0);
Exits: [3]

Block 3
-------
Before: [int x = <undefined>]
Entrances: [0, 2]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): x
Node 2 (0x610000000e70): 1
Node 3 (0x610000000e88): (x = 1)
Node 4 (0x610000000ea0): (x = 1);
Exits: [1]

Block 4
-------
Before: [int x = <undefined>]
Entrances: [1]
Exits: []

about to simplify ST int x; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010420): int x
Node 1 (0x608000010438): int x;
Node 2 (0x608000010450): 3
Node 3 (0x608000010468): switch (3) {
case 0:
(x = 0);
case 1:
(x = 1);
}
Exits: [1, 2, 3]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0, 3]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Exits: [4]

Block 2
-------
Before: [int x = <undefined>]
Entrances: [0]
Node 0 (0x610000000d40): 0
Node 1 (0x610000000d58): x
Node 2 (0x610000000d70): 0
Node 3 (0x610000000d88): (x = 0)
Node 4 (0x610000000da0): (x = 0);
Exits: [3]

Block 3
-------
Before: [int x = <undefined>]
Entrances: [0, 2]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): x
Node 2 (0x610000000e70): 1
Node 3 (0x610000000e88): (x = 1)
Node 4 (0x610000000ea0): (x = 1);
Exits: [1]

Block 4
-------
Before: [int x = <undefined>]
Entrances: [1]
Exits: []

about to simplify EX 3 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010420): int x
Node 1 (0x608000010438): int x;
Node 2 (0x608000010450): 3
Node 3 (0x608000010468): switch (3) {
case 0:
(x = 0);
case 1:
(x = 1);
}
Exits: [1, 2, 3]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0, 3]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half(x)
Node 3 (0x610000000f88): half4(half(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(half(x)))
Node 5 (0x610000000fb8): (sk_FragColor = half4(half(x)));
Exits: [4]

Block 2
-------
Before: [int x = <undefined>]
Entrances: [0]
Node 0 (0x610000000d40): 0
Node 1 (0x610000000d58): x
Node 2 (0x610000000d70): 0
Node 3 (0x610000000d88): (x = 0)
Node 4 (0x610000000da0): (x = 0);
Exits: [3]

Block 3
-------
Before: [int x = <undefined>]
Entrances: [0, 2]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): x
Node 2 (0x610000000e70): 1
Node 3 (0x610000000e88): (x = 1)
Node 4 (0x610000000ea0): (x = 1);
Exits: [1]

Block 4
-------
Before: [int x = <undefined>]
Entrances: [1]
Exits: []

about to simplify ST switch (3) {
case 0:
(x = 0);
case 1:
(x = 1);
} 
simplifying switch
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): int x
Node 1 (0x610000001058): int x;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify ST int x 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify ST int ; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX half(x) 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX half4(half(x)) 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half(x))) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half(x))); 
simplifying expr
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify ST ; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify ST int ; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX half(x) 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX half4(half(x)) 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half(x))) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): x
Node 4 (0x6100000010a0): half(x)
Node 5 (0x6100000010b8): half4(half(x))
Node 6 (0x6100000010d0): (sk_FragColor = half4(half(x)))
Node 7 (0x6100000010e8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [int x = <undefined>]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half(x))); 
simplifying expr
error: 7: 'x' has not been assigned
1 error
