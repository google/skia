### Compilation failed:

Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify ST int r 
simplifying vardecl
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify ST int g 
simplifying vardecl
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify ST int r, g; 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX r 
optimizing varref 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX (r = 1) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify ST (r = 1); 
simplifying expr
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX g 
optimizing varref 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX (g = 0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify ST (g = 0); 
simplifying expr
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): r
Node 13 (0x617000002138): half(r)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(r), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(r), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX r 
optimized to 1 
coerced to 1 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1
Node 13 (0x617000002138): half(1)
Node 14 (0x617000002150): g
Node 15 (0x617000002168): half(g)
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): 1.0
Node 18 (0x6170000021b0): half4(half(1), half(g), 1.0, 1.0)
Node 19 (0x6170000021c8): (sk_FragColor = half4(half(1), half(g), 1.0, 1.0))
Node 20 (0x6170000021e0): (sk_FragColor = half4(half(1), half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX half(1) 
optimized to 1.0 
coerced to 1.0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): g
Node 14 (0x617000002150): half(g)
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): half4(1.0, half(g), 1.0, 1.0)
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, half(g), 1.0, 1.0))
Node 19 (0x6170000021c8): (sk_FragColor = half4(1.0, half(g), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX g 
optimized to 0 
coerced to 0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0
Node 14 (0x617000002150): half(0)
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): 1.0
Node 17 (0x617000002198): half4(1.0, half(0), 1.0, 1.0)
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, half(0), 1.0, 1.0))
Node 19 (0x6170000021c8): (sk_FragColor = half4(1.0, half(0), 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX half(0) 
optimized to 0.0 
coerced to 0.0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x61700=================================================================
==78022==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000019bf0 at pc 0x00010ce4ac05 bp 0x7ffee2ecae10 sp 0x7ffee2ecae08
READ of size 8 at 0x602000019bf0 thread T0
    #0 0x10ce4ac04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10ce4bbad in SkSL::BasicBlock::Node::description() const SkSLCFGGenerator.h:65
    #2 0x10ce4a069 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:79
    #3 0x10ce48fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #4 0x10ceeed91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #5 0x10cefd657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #6 0x10cefa6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #7 0x10cd36108 in main SkSLMain.cpp:258
    #8 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x602000019bf0 is located 0 bytes inside of 16-byte region [0x602000019bf0,0x602000019c00)
freed by thread T0 here:
    #0 0x10ea61c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10cd52f9c in std::__1::_DeallocateCaller::__do_call(void*) new:334
    #2 0x10cd52f40 in std::__1::_DeallocateCaller::__do_deallocate_handle_size(void*, unsigned long) new:292
    #3 0x10d14745c in std::__1::_DeallocateCaller::__do_deallocate_handle_size_align(void*, unsigned long, unsigned long) new:268
    #4 0x10d1473ec in std::__1::__libcpp_deallocate(void*, unsigned long, unsigned long) new:340
    #5 0x10d1a08f1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::deallocate(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1872
    #6 0x10d1a04dc in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::deallocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1594
    #7 0x10d19feab in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:464
    #8 0x10d50029d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #9 0x10d4fa371 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #10 0x10d4fa2a0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #11 0x10cf2a4b4 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #12 0x10d14cda4 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #13 0x10d14aeb1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #14 0x10d14af11 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #15 0x10cf951f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #16 0x10cf94bdd in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #17 0x10cee115d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #18 0x10cee2e24 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719
    #19 0x10cedf4b7 in SkSL::Compiler::simplifyExpression(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:925
    #20 0x10ceeeeff in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1519
    #21 0x10cefd657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #22 0x10cefa6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #23 0x10cd36108 in main SkSLMain.cpp:258
    #24 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10ea6184d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10d145e64 in std::__1::__libcpp_allocate(unsigned long, unsigned long) new:253
    #2 0x10d1a44b1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::allocate(unsigned long, void const*) memory:1869
    #3 0x10d1a3fa8 in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::allocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, unsigned long) memory:1586
    #4 0x10d1a3c0c in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:318
    #5 0x10d1a2d2b in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:317
    #6 0x10d124341 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::reserve(unsigned long) vector:1586
    #7 0x10d149af4 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:55
    #8 0x10d0b64e7 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:54
    #9 0x10d1069eb in std::__1::__unique_if<SkSL::BinaryExpression>::__unique_single std::__1::make_unique<SkSL::BinaryExpression, int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*&>(int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Type const*&) memory:3033
    #10 0x10d0db7b0 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1962
    #11 0x10d0a7c39 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #12 0x10d08ee58 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #13 0x10d08257b in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #14 0x10d0930e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #15 0x10d083a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #16 0x10d0c68cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #17 0x10d12b663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #18 0x10cef9e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #19 0x10cd36108 in main SkSLMain.cpp:258
    #20 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c0400003320: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003330: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003340: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003350: fa fa fd fd fa fa 00 00 fa fa 00 fa fa fa fd fa
  0x1c0400003360: fa fa 00 00 fa fa fd fa fa fa fd fd fa fa 00 00
=>0x1c0400003370: fa fa fd fa fa fa 00 00 fa fa fd fa fa fa[fd]fd
  0x1c0400003380: fa fa fd fd fa fa 00 00 fa fa fd fa fa fa fd fd
  0x1c0400003390: fa fa fd fa fa fa fd fa fa fa fd fa fa fa fd fa
  0x1c04000033a0: fa fa 00 00 fa fa 00 fa fa fa 00 00 fa fa fd fd
  0x1c04000033b0: fa fa 00 00 fa fa fd fd fa fa fd fd fa fa fd fa
  0x1c04000033c0: fa fa fd fa fa fa fd fd fa fa 00 00 fa fa fa fa
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
==78022==ABORTING
0002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX half4(1.0, 0.0, 1.0, 1.0) 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0)); 
simplifying expr
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): int r
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int r, g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify ST int r 
simplifying vardecl
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): int g
Node 2 (0x617000002030): int g;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify ST int g 
simplifying vardecl
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): ;
Node 2 (0x617000002030): int ;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify ST int ; 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): ;
Node 2 (0x617000002030): int ;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX r 
optimizing varref 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): ;
Node 2 (0x617000002030): int ;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): ;
Node 2 (0x617000002030): int ;
Node 3 (0x617000002048): r
Node 4 (0x617000002060): 1
Node 5 (0x617000002078): (r = 1)
Node 6 (0x617000002090): (r = 1);
Node 7 (0x6170000020a8): g
Node 8 (0x6170000020c0): 0
Node 9 (0x6170000020d8): (g = 0)
Node 10 (0x6170000020f0): (g = 0);
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): 1.0
Node 13 (0x617000002138): 0.0
Node 14 (0x617000002150): 1.0
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): half4(1.0, 0.0, 1.0, 1.0)
Node 17 (0x617000002198): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0))
Node 18 (0x6170000021b0): (sk_FragColor = half4(1.0, 0.0, 1.0, 1.0));
Exits: [1]

Block 1
-------
Before: [int g = 0, int r = 1]
Entrances: [0]
Exits: []

about to simplify EX (r = 1) 
optimizing binary 
deadass? update=1 rescan=0 
deadass! update=1 rescan=0 
delete_left update=1 rescan=0 
Block 0
-------
Before: [int g = <undefined>, int r = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): ;
Node 2 (0x617000002030): int ;
