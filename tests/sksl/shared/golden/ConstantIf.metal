### Compilation failed:

Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify ST int x 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify ST int x; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX (x = 1) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify ST (x = 1); 
simplifying expr
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX 2 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX (x = 2) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify ST (x = 2); 
simplifying expr
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX 5 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX (x = 5) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify ST (x = 5); 
simplifying expr
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor.x 
optimizing swiz 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): x
Node 17 (0x617000002198): half(x)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(x))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(x));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to 5 
coerced to 5 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): 5
Node 17 (0x617000002198): half(5)
Node 18 (0x6170000021b0): (sk_FragColor.x = half(5))
Node 19 (0x6170000021c8): (sk_FragColor.x = half(5));
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX half(5) 
optimized to 5.0 
coerced to 5.0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): 5.0
Node 17 (0x617000002198): (sk_FragColor.x = 5.0)
Node 18 (0x6170000021b0): (sk_FragColor.x = 5.0);
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor.x = 5.0) 
optimizing binary 
deadass? update=1 rescan=0 
Block =================================================================
==77507==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000019b90 at pc 0x000100c1dc05 bp 0x7ffeef0f7e30 sp 0x7ffeef0f7e28
READ of size 8 at 0x602000019b90 thread T0
    #0 0x100c1dc04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x100c1ebad in SkSL::BasicBlock::Node::description() const SkSLCFGGenerator.h:65
    #2 0x100c1d069 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:79
    #3 0x100c1bfcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #4 0x100cc1d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #5 0x100cd0657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #6 0x100ccd6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #7 0x100b09108 in main SkSLMain.cpp:258
    #8 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x602000019b90 is located 0 bytes inside of 16-byte region [0x602000019b90,0x602000019ba0)
freed by thread T0 here:
    #0 0x102834c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x100b25f9c in std::__1::_DeallocateCaller::__do_call(void*) new:334
    #2 0x100b25f40 in std::__1::_DeallocateCaller::__do_deallocate_handle_size(void*, unsigned long) new:292
    #3 0x100f1a45c in std::__1::_DeallocateCaller::__do_deallocate_handle_size_align(void*, unsigned long, unsigned long) new:268
    #4 0x100f1a3ec in std::__1::__libcpp_deallocate(void*, unsigned long, unsigned long) new:340
    #5 0x100f738f1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::deallocate(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1872
    #6 0x100f734dc in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::deallocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1594
    #7 0x100f72eab in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:464
    #8 0x1012d329d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #9 0x1012cd371 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #10 0x1012cd2a0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #11 0x100cfd4b4 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #12 0x100f1fda4 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #13 0x100f1deb1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #14 0x100f1df11 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #15 0x100d681f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #16 0x100d67bdd in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #17 0x100cb415d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #18 0x100cb5e24 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719
    #19 0x100cb24b7 in SkSL::Compiler::simplifyExpression(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:925
    #20 0x100cc1eff in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1519
    #21 0x100cd0657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #22 0x100ccd6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #23 0x100b09108 in main SkSLMain.cpp:258
    #24 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10283484d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x100f18e64 in std::__1::__libcpp_allocate(unsigned long, unsigned long) new:253
    #2 0x100f774b1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::allocate(unsigned long, void const*) memory:1869
    #3 0x100f76fa8 in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::allocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, unsigned long) memory:1586
    #4 0x100f76c0c in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:318
    #5 0x100f75d2b in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:317
    #6 0x100ef7341 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::reserve(unsigned long) vector:1586
    #7 0x100f1caf4 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:55
    #8 0x100e894e7 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:54
    #9 0x100ed99eb in std::__1::__unique_if<SkSL::BinaryExpression>::__unique_single std::__1::make_unique<SkSL::BinaryExpression, int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*&>(int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Type const*&) memory:3033
    #10 0x100eae7b0 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1962
    #11 0x100e7ac39 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #12 0x100e61e58 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #13 0x100e5557b in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #14 0x100e660e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #15 0x100e57bf6 in SkSL::IRGenerator::convertIf(SkSL::ASTNode const&) SkSLIRGenerator.cpp:522
    #16 0x100e551e3 in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:224
    #17 0x100e660e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #18 0x100e56a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #19 0x100e998cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #20 0x100efe663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #21 0x100ccce67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #22 0x100b09108 in main SkSLMain.cpp:258
    #23 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c0400003320: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003330: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003340: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003350: fa fa fd fd fa fa 00 00 fa fa 00 fa fa fa fd fa
  0x1c0400003360: fa fa 00 00 fa fa 00 fa fa fa 00 fa fa fa fd fa
=>0x1c0400003370: fa fa[fd]fd fa fa fd fd fa fa 00 00 fa fa fd fd
  0x1c0400003380: fa fa fd fd fa fa 00 00 fa fa fd fd fa fa fd fa
  0x1c0400003390: fa fa 04 fa fa fa fd fa fa fa 00 00 fa fa 00 fa
  0x1c04000033a0: fa fa 00 00 fa fa fd fd fa fa 00 00 fa fa fd fd
  0x1c04000033b0: fa fa fd fd fa fa fd fa fa fa fd fd fa fa 00 00
  0x1c04000033c0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==77507==ABORTING
0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): 5.0
Node 17 (0x617000002198): (sk_FragColor.x = 5.0)
Node 18 (0x6170000021b0): (sk_FragColor.x = 5.0);
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor.x = 5.0); 
simplifying expr
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): int x
Node 1 (0x617000002018): int x;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): 5.0
Node 17 (0x617000002198): (sk_FragColor.x = 5.0)
Node 18 (0x6170000021b0): (sk_FragColor.x = 5.0);
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify ST int x 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): int ;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): 5.0
Node 17 (0x617000002198): (sk_FragColor.x = 5.0)
Node 18 (0x6170000021b0): (sk_FragColor.x = 5.0);
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify ST int ; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): int ;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): 5.0
Node 17 (0x617000002198): (sk_FragColor.x = 5.0)
Node 18 (0x6170000021b0): (sk_FragColor.x = 5.0);
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): int ;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): 5.0
Node 17 (0x617000002198): (sk_FragColor.x = 5.0)
Node 18 (0x6170000021b0): (sk_FragColor.x = 5.0);
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): int ;
Node 2 (0x617000002030): x
Node 3 (0x617000002048): 1
Node 4 (0x617000002060): (x = 1)
Node 5 (0x617000002078): (x = 1);
Node 6 (0x617000002090): x
Node 7 (0x6170000020a8): 2
Node 8 (0x6170000020c0): (x = 2)
Node 9 (0x6170000020d8): (x = 2);
Node 10 (0x6170000020f0): x
Node 11 (0x617000002108): 5
Node 12 (0x617000002120): (x = 5)
Node 13 (0x617000002138): (x = 5);
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): sk_FragColor.x
Node 16 (0x617000002180): 5.0
Node 17 (0x617000002198): (sk_FragColor.x = 5.0)
Node 18 (0x6170000021b0): (sk_FragColor.x = 5.0);
Exits: [1]

Block 1
-------
Before: [int x = 5]
Entrances: [0]
Exits: []

about to simplify EX (x = 1) 
optimizing binary 
deadass? update=1 rescan=0 
deadass! update=1 rescan=0 
delete_left update=1 rescan=0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): int ;
