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
case 1:
(x = 1.0);
}
Exits: [1, 2, 3]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3]
Node 0 (0x610000001040): sk_FragColor
Node 1 (0x610000001058): x
Node 2 (0x610000001070): half(x)
Node 3 (0x610000001088): half4(half(x))
Node 4 (0x6100000010a0): (sk_FragColor = half4(half(x)))
Node 5 (0x6100000010b8): (sk_FragColor = half4(half(x)));
Exits: [4]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x610000000e40): 0
Node 1 (0x610000000e58): x
Node 2 (0x610000000e70): 0.0
Node 3 (0x610000000e88): (x = 0.0)
Node 4 (0x610000000ea0): (x = 0.0);
Exits: [3]

Block 3
-------
Before: [float x = <defined>]
Entrances: [0, 2]
Node 0 (0x610000000f40): 1
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): 1.0
Node 3 (0x610000000f88): (x = 1.0)
Node 4 (0x610000000fa0): (x = 1.0);
Exits: [1]

Block 4
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
case 1:
(x = 1.0);
}
Exits: [1, 2, 3]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3]
Node 0 (0x610000001040): sk_FragColor
Node 1 (0x610000001058): x
Node 2 (0x610000001070): half(x)
Node 3 (0x610000001088): half4(half(x))
Node 4 (0x6100000010a0): (sk_FragColor = half4(half(x)))
Node 5 (0x6100000010b8): (sk_FragColor = half4(half(x)));
Exits: [4]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x610000000e40): 0
Node 1 (0x610000000e58): x
Node 2 (0x610000000e70): 0.0
Node 3 (0x610000000e88): (x = 0.0)
Node 4 (0x610000000ea0): (x = 0.0);
Exits: [3]

Block 3
-------
Before: [float x = <defined>]
Entrances: [0, 2]
Node 0 (0x610000000f40): 1
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): 1.0
Node 3 (0x610000000f88): (x = 1.0)
Node 4 (0x610000000fa0): (x = 1.0);
Exits: [1]

Block 4
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
case 1:
(x = 1.0);
}
Exits: [1, 2, 3]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3]
Node 0 (0x610000001040): sk_FragColor
Node 1 (0x610000001058): x
Node 2 (0x610000001070): half(x)
Node 3 (0x610000001088): half4(half(x))
Node 4 (0x6100000010a0): (sk_FragColor = half4(half(x)))
Node 5 (0x6100000010b8): (sk_FragColor = half4(half(x)));
Exits: [4]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x610000000e40): 0
Node 1 (0x610000000e58): x
Node 2 (0x610000000e70): 0.0
Node 3 (0x610000000e88): (x = 0.0)
Node 4 (0x610000000ea0): (x = 0.0);
Exits: [3]

Block 3
-------
Before: [float x = <defined>]
Entrances: [0, 2]
Node 0 (0x610000000f40): 1
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): 1.0
Node 3 (0x610000000f88): (x = 1.0)
Node 4 (0x610000000fa0): (x = 1.0);
Exits: [1]

Block 4
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
case 1:
(x = 1.0);
}
Exits: [1, 2, 3]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3]
Node 0 (0x610000001040): sk_FragColor
Node 1 (0x610000001058): x
Node 2 (0x610000001070): half(x)
Node 3 (0x610000001088): half4(half(x))
Node 4 (0x6100000010a0): (sk_FragColor = half4(half(x)))
Node 5 (0x6100000010b8): (sk_FragColor = half4(half(x)));
Exits: [4]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x610000000e40): 0
Node 1 (0x610000000e58): x
Node 2 (0x610000000e70): 0.0
Node 3 (0x610000000e88): (x = 0.0)
Node 4 (0x610000000ea0): (x = 0.0);
Exits: [3]

Block 3
-------
Before: [float x = <defined>]
Entrances: [0, 2]
Node 0 (0x610000000f40): 1
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): 1.0
Node 3 (0x610000000f88): (x = 1.0)
Node 4 (0x610000000fa0): (x = 1.0);
Exits: [1]

Block 4
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
case 1:
(x = 1.0);
}
Exits: [1, 2, 3]

Block 1
-------
Before: [float x = <defined>]
Entrances: [0, 3]
Node 0 (0x610000001040): sk_FragColor
Node 1 (0x610000001058): x
Node 2 (0x610000001070): half(x)
Node 3 (0x610000001088): half4(half(x))
Node 4 (0x6100000010a0): (sk_FragColor = half4(half(x)))
Node 5 (0x6100000010b8): (sk_FragColor = half4(half(x)));
Exits: [4]

Block 2
-------
Before: [float x = 0.0]
Entrances: [0]
Node 0 (0x610000000e40): 0
Node 1 (0x610000000e58): x
Node 2 (0x610000000e70): 0.0
Node 3 (0x610000000e88): (x = 0.0)
Node 4 (0x610000000ea0): (x = 0.0);
Exits: [3]

Block 3
-------
Before: [float x = <defined>]
Entrances: [0, 2]
Node 0 (0x610000000f40): 1
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): 1.0
Node 3 (0x610000000f88): (x = 1.0)
Node 4 (0x610000000fa0): (x = 1.0);
Exits: [1]

Block 4
-------
Before: [float x = <defined>]
Entrances: [1]
Exits: []

about to simplify ST switch (0) {
case 0:
(x = 0.0);
case 1:
(x = 1.0);
} 
simplifying switch
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify ST float x = 0.0 
simplifying vardecl
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify ST float x = 0.0; 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX (x = 0.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify ST (x = 0.0); 
simplifying expr
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX (x = 1.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify ST (x = 1.0); 
simplifying expr
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): x
Node 13 (0x617000002138): half(x)
Node 14 (0x617000002150): half4(half(x))
Node 15 (0x617000002168): (sk_FragColor = half4(half(x)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-----=================================================================
==77849==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000019bf0 at pc 0x000110066c05 bp 0x7ffedfcaee10 sp 0x7ffedfcaee08
READ of size 8 at 0x602000019bf0 thread T0
    #0 0x110066c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x110067bad in SkSL::BasicBlock::Node::description() const SkSLCFGGenerator.h:65
    #2 0x110066069 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:79
    #3 0x110064fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #4 0x11010ad91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #5 0x110119657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #6 0x1101166e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #7 0x10ff51886 in main SkSLMain.cpp:242
    #8 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x602000019bf0 is located 0 bytes inside of 16-byte region [0x602000019bf0,0x602000019c00)
freed by thread T0 here:
    #0 0x111c82c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10ff6ef9c in std::__1::_DeallocateCaller::__do_call(void*) new:334
    #2 0x10ff6ef40 in std::__1::_DeallocateCaller::__do_deallocate_handle_size(void*, unsigned long) new:292
    #3 0x11036345c in std::__1::_DeallocateCaller::__do_deallocate_handle_size_align(void*, unsigned long, unsigned long) new:268
    #4 0x1103633ec in std::__1::__libcpp_deallocate(void*, unsigned long, unsigned long) new:340
    #5 0x1103bc8f1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::deallocate(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1872
    #6 0x1103bc4dc in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::deallocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1594
    #7 0x1103bbeab in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:464
    #8 0x11071c29d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #9 0x110716371 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #10 0x1107162a0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #11 0x1101464b4 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #12 0x110368da4 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #13 0x110366eb1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #14 0x110366f11 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #15 0x1101b11f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #16 0x1101b0bdd in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #17 0x1100fd15d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #18 0x1100fee24 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719
    #19 0x1100fb4b7 in SkSL::Compiler::simplifyExpression(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:925
    #20 0x11010aeff in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1519
    #21 0x110119657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #22 0x1101166e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #23 0x10ff51886 in main SkSLMain.cpp:242
    #24 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x111c8284d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x110361e64 in std::__1::__libcpp_allocate(unsigned long, unsigned long) new:253
    #2 0x1103c04b1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::allocate(unsigned long, void const*) memory:1869
    #3 0x1103bffa8 in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::allocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, unsigned long) memory:1586
    #4 0x1103bfc0c in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:318
    #5 0x1103bed2b in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:317
    #6 0x110340341 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::reserve(unsigned long) vector:1586
    #7 0x110365af4 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:55
    #8 0x1102d24e7 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:54
    #9 0x1103229eb in std::__1::__unique_if<SkSL::BinaryExpression>::__unique_single std::__1::make_unique<SkSL::BinaryExpression, int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*&>(int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Type const*&) memory:3033
    #10 0x1102f77b0 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1962
    #11 0x1102c3c39 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #12 0x1102aae58 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #13 0x11029e57b in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #14 0x1102af0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #15 0x1102a72a0 in SkSL::IRGenerator::convertSwitch(SkSL::ASTNode const&) SkSLIRGenerator.cpp:682
    #16 0x11029e377 in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:232
    #17 0x1102af0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #18 0x11029fa03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #19 0x1102e28cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #20 0x110347663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #21 0x110115e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #22 0x10ff51886 in main SkSLMain.cpp:242
    #23 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c0400003320: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003330: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003340: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003350: fa fa fd fd fa fa 00 00 fa fa 00 fa fa fa fd fa
  0x1c0400003360: fa fa fd fa fa fa 00 00 fa fa 00 fa fa fa 00 fa
=>0x1c0400003370: fa fa fd fa fa fa fd fd fa fa fd fa fa fa[fd]fd
  0x1c0400003380: fa fa fd fa fa fa fd fa fa fa fd fa fa fa 00 00
  0x1c0400003390: fa fa fd fa fa fa fd fd fa fa fd fd fa fa 00 fa
  0x1c04000033a0: fa fa 00 fa fa fa 00 00 fa fa 00 fa fa fa fd fa
  0x1c04000033b0: fa fa fd fd fa fa fd fd fa fa fd fd fa fa fd fd
  0x1c04000033c0: fa fa fd fd fa fa fd fd fa fa fd fd fa fa fd fd
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
==77849==ABORTING
--
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to 1.0 
coerced to 1.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): half(1.0)
Node 14 (0x617000002150): half4(half(1.0))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX half(1.0) 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): half(1.0)
Node 14 (0x617000002150): half4(half(1.0))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX half4(half(1.0)) 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): half(1.0)
Node 14 (0x617000002150): half4(half(1.0))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half(1.0))) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): half(1.0)
Node 14 (0x617000002150): half4(half(1.0))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half(1.0))); 
simplifying expr
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): half(1.0)
Node 14 (0x617000002150): half4(half(1.0))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0.0
Node 1 (0x617000002018): float x = 0.0
Node 2 (0x617000002030): float x = 0.0;
Node 3 (0x617000002048): x
Node 4 (0x617000002060): 0.0
Node 5 (0x617000002078): (x = 0.0)
Node 6 (0x617000002090): (x = 0.0);
Node 7 (0x6170000020a8): x
Node 8 (0x6170000020c0): 1.0
Node 9 (0x6170000020d8): (x = 1.0)
Node 10 (0x6170000020f0): (x = 1.0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): half(1.0)
Node 14 (0x617000002150): half4(half(1.0))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)))
Node 16 (0x617000002180): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify ST float x = 0.0 
simplifying vardecl
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): float ;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 0.0
Node 4 (0x617000002060): (x = 0.0)
Node 5 (0x617000002078): (x = 0.0);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 1.0
Node 8 (0x6170000020c0): (x = 1.0)
Node 9 (0x6170000020d8): (x = 1.0);
Node 10 (0x6170000020f0): sk_FragColor
Node 11 (0x617000002108): 1.0
Node 12 (0x617000002120): half(1.0)
Node 13 (0x617000002138): half4(half(1.0))
Node 14 (0x617000002150): (sk_FragColor = half4(half(1.0)))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify ST float ; 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): float ;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 0.0
Node 4 (0x617000002060): (x = 0.0)
Node 5 (0x617000002078): (x = 0.0);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 1.0
Node 8 (0x6170000020c0): (x = 1.0)
Node 9 (0x6170000020d8): (x = 1.0);
Node 10 (0x6170000020f0): sk_FragColor
Node 11 (0x617000002108): 1.0
Node 12 (0x617000002120): half(1.0)
Node 13 (0x617000002138): half4(half(1.0))
Node 14 (0x617000002150): (sk_FragColor = half4(half(1.0)))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): float ;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 0.0
Node 4 (0x617000002060): (x = 0.0)
Node 5 (0x617000002078): (x = 0.0);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 1.0
Node 8 (0x6170000020c0): (x = 1.0)
Node 9 (0x6170000020d8): (x = 1.0);
Node 10 (0x6170000020f0): sk_FragColor
Node 11 (0x617000002108): 1.0
Node 12 (0x617000002120): half(1.0)
Node 13 (0x617000002138): half4(half(1.0))
Node 14 (0x617000002150): (sk_FragColor = half4(half(1.0)))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): float ;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 0.0
Node 4 (0x617000002060): (x = 0.0)
Node 5 (0x617000002078): (x = 0.0);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 1.0
Node 8 (0x6170000020c0): (x = 1.0)
Node 9 (0x6170000020d8): (x = 1.0);
Node 10 (0x6170000020f0): sk_FragColor
Node 11 (0x617000002108): 1.0
Node 12 (0x617000002120): half(1.0)
Node 13 (0x617000002138): half4(half(1.0))
Node 14 (0x617000002150): (sk_FragColor = half4(half(1.0)))
Node 15 (0x617000002168): (sk_FragColor = half4(half(1.0)));
Exits: [1]

Block 1
-------
Before: [float x = 1.0]
Entrances: [0]
Exits: []

about to simplify EX (x = 0.0) 
optimizing binary 
deadass? update=1 rescan=0 
deadass! update=1 rescan=0 
delete_left update=1 rescan=0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): float ;
