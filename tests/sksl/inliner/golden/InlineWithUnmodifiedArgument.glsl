### Compilation failed:

Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000108a0): x
Node 1 (0x6080000108b8): 2.0
Node 2 (0x6080000108d0): (x * 2.0)
Node 3 (0x6080000108e8): return (x * 2.0);
Exits: []

Block 1
-------
Before: []
Entrances: []
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000108a0): x
Node 1 (0x6080000108b8): 2.0
Node 2 (0x6080000108d0): (x * 2.0)
Node 3 (0x6080000108e8): return (x * 2.0);
Exits: []

Block 1
-------
Before: []
Entrances: []
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000108a0): x
Node 1 (0x6080000108b8): 2.0
Node 2 (0x6080000108d0): (x * 2.0)
Node 3 (0x6080000108e8): return (x * 2.0);
Exits: []

Block 1
-------
Before: []
Entrances: []
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify EX (x * 2.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000108a0): x
Node 1 (0x6080000108b8): 2.0
Node 2 (0x6080000108d0): (x * 2.0)
Node 3 (0x6080000108e8): return (x * 2.0);
Exits: []

Block 1
-------
Before: []
Entrances: []
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify ST return (x * 2.0); 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002000): half _0_basic
Node 1 (0x617000002018): half _0_basic;
Node 2 (0x617000002030): 1.0
Node 3 (0x617000002048): half _1_x = 1.0
Node 4 (0x617000002060): half _1_x = 1.0;
Node 5 (0x617000002078): _0_basic
Node 6 (0x617000002090): _1_x
Node 7 (0x6170000020a8): 2.0
Node 8 (0x6170000020c0): (_1_x * 2.0)
Node 9 (0x6170000020d8): (_0_basic = (_1_x * 2.0))
Node 10 (0x6170000020f0): (_0_basic = (_1_x * 2.0));
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): sk_FragColor.x
Node 13 (0x617000002138): _0_basic
Node 14 (0x617000002150): (sk_FragColor.x = _0_basic)
Node 15 (0x617000002168): (sk_FragColor.x = _0_basic);
Node 16 (0x617000002180): 2.0
Node 17 (0x617000002198): half y = 2.0
Node 18 (0x6170000021b0): half y = 2.0;
Node 19 (0x6170000021c8): half _2_basic
Node 20 (0x6170000021e0): half _2_basic;
Node 21 (0x6170000021f8): _2_basic
Node 22 (0x617000002210): y
Node 23 (0x617000002228): 2.0
Node 24 (0x617000002240): (y * 2.0)
Node 25 (0x617000002258): (_2_basic = (y * 2.0))
Node 26 (0x617000002270): (_2_basic = (y * 2.0));
Node 27 (0x617000002288): sk_FragColor
Node 28 (0x6170000022a0): sk_FragColor.y
Node 29 (0x6170000022b8): _2_basic
Node 30 (0x6170000022d0): (sk_FragColor.y = _2_basic)
Node 31 (0x6170000022e8): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = (_1_x * 2.0)]
Entrances: [0]
Exits: []

about to simplify ST half _0_basic 
simplifying vardecl
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002000): half _0_basic
Node 1 (0x617000002018): half _0_basic;
Node 2 (0x617000002030): 1.0
Node 3 (0x617000002048): half _1_x = 1.0
Node 4 (0x617000002060): half _1_x = 1.0;
Node 5 (0x617000002078): _0_basic
Node 6 (0x617000002090): _1_x
Node 7 (0x6170000020a8): 2.0
Node 8 (0x6170000020c0): (_1_x * 2.0)
Node 9 (0x6170000020d8): (_0_basic = (_1_x * 2.0))
Node 10 (0x6170000020f0): (_0_basic = (_1_x * 2.0));
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): sk_FragColor.x
Node 13 (0x617000002138): _0_basic
Node 14 (0x617000002150): (sk_FragColor.x = _0_basic)
Node 15 (0x617000002168): (sk_FragColor.x = _0_basic);
Node 16 (0x617000002180): 2.0
Node 17 (0x617000002198): half y = 2.0
Node 18 (0x6170000021b0): half y = 2.0;
Node 19 (0x6170000021c8): half _2_basic
Node 20 (0x6170000021e0): half _2_basic;
Node 21 (0x6170000021f8): _2_basic
Node 22 (0x617000002210): y
Node 23 (0x617000002228): 2.0
Node 24 (0x617000002240): (y * 2.0)
Node 25 (0x617000002258): (_2_basic = (y * 2.0))
Node 26 (0x617000002270): (_2_basic = (y * 2.0));
Node 27 (0x617000002288): sk_FragColor
Node 28 (0x6170000022a0): sk_FragColor.y
Node 29 (0x6170000022b8): _2_basic
Node 30 (0x6170000022d0): (sk_FragColor.y = _2_basic)
Node 31 (0x6170000022e8): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = (_1_x * 2.0)]
Entrances: [0]
Exits: []

about to simplify ST half _0_basic; 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002000): half _0_basic
Node 1 (0x617000002018): half _0_basic;
Node 2 (0x617000002030): 1.0
Node 3 (0x617000002048): half _1_x = 1.0
Node 4 (0x617000002060): half _1_x = 1.0;
Node 5 (0x617000002078): _0_basic
Node 6 (0x617000002090): _1_x
Node 7 (0x6170000020a8): 2.0
Node 8 (0x6170000020c0): (_1_x * 2.0)
Node 9 (0x6170000020d8): (_0_basic = (_1_x * 2.0))
Node 10 (0x6170000020f0): (_0_basic = (_1_x * 2.0));
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): sk_FragColor.x
Node 13 (0x617000002138): _0_basic
Node 14 (0x617000002150): (sk_FragColor.x = _0_basic)
Node 15 (0x617000002168): (sk_FragColor.x = _0_basic);
Node 16 (0x617000002180): 2.0
Node 17 (0x617000002198): half y = 2.0
Node 18 (0x6170000021b0): half y = 2.0;
Node 19 (0x6170000021c8): half _2_basic
Node 20 (0x6170000021e0): half _2_basic;
Node 21 (0x6170000021f8): _2_basic
Node 22 (0x617000002210): y
Node 23 (0x617000002228): 2.0
Node 24 (0x617000002240): (y * 2.0)
Node 25 (0x617000002258): (_2_basic = (y * 2.0))
Node 26 (0x617000002270): (_2_basic = (y * 2.0));
Node 27 (0x617000002288): sk_FragColor
Node 28 (0x6170000022a0): sk_FragColor.y
Node 29 (0x6170000022b8): _2_basic
Node 30 (0x6170000022d0): (sk_FragColor.y = _2_basic)
Node 31 (0x6170000022e8): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = (_1_x * 2.0)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002000): half _0_basic
Node 1 (0x617000002018): half _0_basic;
Node 2 (0x617000002030): 1.0
Node 3 (0x617000002048): half _1_x = 1.0
Node 4 (0x617000002060): half _1_x = 1.0;
Node 5 (0x617000002078): _0_basic
Node 6 (0x617000002090): _1_x
Node 7 (0x6170000020a8): 2.0
Node 8 (0x6170000020c0): (_1_x * 2.0)
Node 9 (0x6170000020d8): (_0_basic = (_1_x * 2.0))
Node 10 (0x6170000020f0): (_0_basic = (_1_x * 2.0));
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): sk_FragColor.x
Node 13 (0x617000002138): _0_basic
Node 14 (0x617000002150): (sk_FragColor.x = _0_basic)
Node 15 (0x617000002168): (sk_FragColor.x = _0_basic);
Node 16 (0x617000002180): 2.0
Node 17 (0x617000002198): half y = 2.0
Node 18 (0x6170000021b0): half y = 2.0;
Node 19 (0x6170000021c8): half _2_basic
Node 20 (0x6170000021e0): half _2_basic;
Node 21 (0x6170000021f8): _2_basic
Node 22 (0x617000002210): y
Node 23 (0x617000002228): 2.0
Node 24 (0x617000002240): (y * 2.0)
Node 25 (0x617000002258): (_2_basic = (y * 2.0))
Node 26 (0x617000002270): (_2_basic = (y * 2.0));
Node 27 (0x617000002288): sk_FragColor
Node 28 (0x6170000022a0): sk_FragColor.y
Node 29 (0x6170000022b8): _2_basic
Node 30 (0x6170000022d0): (sk_FragColor.y = _2_basic)
Node 31 (0x6170000022e8): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = (_1_x * 2.0)]
Entrances: [0]
Exits: []

about to simplify ST half _1_x = 1.0 
simplifying vardecl
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002000): half _0_basic
Node 1 (0x617000002018): half _0_basic;
Node 2 (0x617000002030): 1.0
Node 3 (0x617000002048): half _1_x = 1.0
Node 4 (0x617000002060): half _1_x = 1.0;
Node 5 (0x617000002078): _0_basic
Node 6 (0x617000002090): _1_x
Node 7 (0x6170000020a8): 2.0
Node 8 (0x6170000020c0): (_1_x * 2.0)
Node 9 (0x6170000020d8): (_0_basic = (_1_x * 2.0))
Node 10 (0x6170000020f0): (_0_basic = (_1_x * 2.0));
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): sk_FragColor.x
Node 13 (0x617000002138): _0_basic
Node 14 (0x617000002150): (sk_FragColor.x = _0_basic)
Node 15 (0x617000002168): (sk_FragColor.x = _0_basic);
Node 16 (0x617000002180): 2.0
Node 17 (0x617000002198): half y = 2.0
Node 18 (0x6170000021b0): half y = 2.0;
Node 19 (0x6170000021c8): half _2_basic
Node 20 (0x6170000021e0): half _2_basic;
Node 21 (0x6170000021f8): _2_basic
Node 22 (0x617000002210): y
Node 23 (0x617000002228): 2.0
Node 24 (0x617000002240): (y * 2.0)
Node 25 (0x617000002258): (_2_basic = (y * 2.0))
Node 26 (0x617000002270): (_2_basic = (y * 2.0));
Node 27 (0x617000002288): sk_FragColor
Node 28 (0x6170000022a0): sk_FragColor.y
Node 29 (0x6170000022b8): _2_basic
Node 30 (0x6170000022d0): (sk_FragColor.y = _2_basic)
Node 31 (0x6170000022e8): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = (_1_x * 2.0)]
Entrances: [0]
Exits: []

about to simplify ST half _1_x = 1.0; 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002000): half _0_basic
Node 1 (0x617000002018): half _0_basic;
Node 2 (0x617000002030): 1.0
Node 3 (0x617000002048): half _1_x = 1.0
Node 4 (0x617000002060): half _1_x = 1.0;
Node 5 (0x617000002078): _0_basic
Node 6 (0x617000002090): _1_x
Node 7 (0x6170000020a8): 2.0
Node 8 (0x6170000020c0): (_1_x * 2.0)
Node 9 (0x6170000020d8): (_0_basic = (_1_x * 2.0))
Node 10 (0x6170000020f0): (_0_basic = (_1_x * 2.0));
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): sk_FragColor.x
Node 13 (0x617000002138): _0_basic
Node 14 (0x617000002150): (sk_FragColor.x = _0_basic)
Node 15 (0x617000002168): (sk_FragColor.x = _0_basic);
Node 16 (0x617000002180): 2.0
Node 17 (0x617000002198): half y = 2.0
Node 18 (0x6170000021b0): half y = 2.0;
Node 19 (0x6170000021c8): half _2_basic
Node 20 (0x6170000021e0): half _2_basic;
Node 21 (0x6170000021f8): _2_basic
Node 22 (0x617000002210): y
Node 23 (0x617000002228): 2.0
Node 24 (0x617000002240): (y * 2.0)
Node 25 (0x617000002258): (_2_basic = (y * 2.0))
Node 26 (0x617000002270): (_2_basic = (y * 2.0));
Node 27 (0x617000002288): sk_FragColor
Node 28 (0x6170000022a0): sk_FragColor.y
Node 29 (0x6170000022b8): _2_basic
Node 30 (0x6170000022d0): (sk_FragColor.y = _2_basic)
Node 31 (0x6170000022e8): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = (_1_x * 2.0)]
Entrances: [0]
Exits: []

about to simplify EX _0_basic 
optimizing varref 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002000): half _0_basic
Node 1 (0x617000002018): half _0_basic;
Node 2 (0x617000002030): 1.0
Node 3 (0x617000002048): half _1_x = 1.0
Node 4 (0x617000002060): half _1_x = 1.0;
Node 5 (0x617000002078): _0_basic
Node 6 (0x617000002090): _1_x
Node 7 (0x6170000020a8): 2.0
Node 8 (0x6170000020c0): (_1_x * 2.0)
Node 9 (0x6170000020d8): (_0_basic = (_1_x * 2.0))
Node 10 (0x6170000020f0): (_0_basic = (_1_x * 2.0));
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): sk_FragColor.x
Node 13 (0x617000002138): _0_basic
Node 14 (0x617000002150): (sk_FragColor.x = _0_basic)
Node 15 (0x617000002168): (sk_FragColor.x = _0_basic);
Node 16 (0x617000002180): 2.0
Node 17 (0x617000002198): half y = 2.0
Node 18 (0x6170000021b0): half y = 2.0;
Node 19 (0x6170000021c8): half _2_basic
Node 20 (0x6170000021e0): half _2_basic;
Node 21 (0x6170000021f8): _2_basic
Node 22 (0x617000002210): y
Node 23 (0x617000002228): 2.0
Node 24 (0x617000002240): (y * 2.0)
Node 25 (0x617000002258): (_2_basic = (y * 2.0))
Node 26 (0x617000002270): (_2_basic = (y * 2.0));
Node 27 (0x617000002288): sk_FragColor
Node 28 (0x6170000022a0): sk_FragColor.y
Node 29 (0x6170000022b8): _2_basic
Node 30 (0x6170000022d0): (sk_FragColor.y = _2_basic)
Node 31 (0x6170000022e8): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = (_1_x * 2.0)]
Entrances: [0]
Exits: []

about to simplify EX _1_x 
optimized to 1.0 
coerced to 1.0 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002000): half _0_basic
Node 1 (0x617000002018): half _0_basic;
Node 2 (0x617000002030): 1.0
Node 3 (0x617000002048): half _1_x = 1.0
Node 4 (0x617000002060): half _1_x = 1.0;
Node 5 (0x617000002078): _0_basic
Node 6 (0x617000002090): 1.0
Node 7 (0x6170000020a8): 2.0
Node 8 (0x6170000020c0): (1.0 * 2.0)
Node 9 (0x6170000020d8): (_0_basic = (1.0 * 2.0))
Node 10 (0x6170000020f0): (_0_basic = (1.0 * 2.0));
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): sk_FragColor.x
Node 13 (0x617000002138): _0_basic
Node 14 (0x617000002150): (sk_FragColor.x = _0_basic)
Node 15 (0x617000002168): (sk_FragColor.x = _0_basic);
Node 16 (0x617000002180): 2.0
Node 17 (0x617000002198): half y = 2.0
Node 18 (0x6170000021b0): half y = 2.0;
Node 19 (0x6170000021c8): half _2_basic
Node 20 (0x6170000021e0): half _2_basic;
Node 21 (0x6170000021f8): _2_basic
Node 22 (0x617000002210): y
Node 23 (0x617000002228): 2.0
Node 24 (0x617000002240): (y * 2.0)
Node 25 (0x617000002258): (_2_basic = (y * 2.0))
Node 26 (0x617000002270): (_2_basic = (y * 2.0));
Node 27 (0x617000002288): sk_FragColor
Node 28 (0x6170000022a0): sk_FragColor.y
Node 29 (0x6170000022b8): _2_basic
Node 30 (0x6170000022d0): (sk_FragColor.y = _2_basic)
Node 31 (0x6170000022e8): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = (1.0 * 2.0)]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002000): half _0_basic
Node 1 (0x617000002018): half _0_basic;
Node 2 (0x617000002030): 1.0
Node 3 (0x617000002048): half _1_x = 1.0
Node 4 (0x617000002060): half _1_x = 1.0;
Node 5 (0x617000002078): _0_basic
Node 6 (0x617000002090): 1.0
Node 7 (0x6170000020a8): 2.0
Node 8 (0x6170000020c0): (1.0 * 2.0)
Node 9 (0x6170000020d8): (_0_basic = (1.0 * 2.0))
Node 10 (0x6170000020f0): (_0_basic = (1.0 * 2.0));
Node 11 (0x617000002108): sk_FragColor
Node 12 (0x617000002120): sk_FragColor.x
Node 13 (0x617000002138): _0_basic
Node 14 (0x617000002150): (sk_FragColor.x = _0_basic)
Node 15 (0x617000002168): (sk_FragColor.x = _0_basic);
Node 16 (0x617000002180): 2.0
Node 17 (0x617000002198): half y = 2.0
Node 18 (0x6170000021b0): half y = 2.0;
Node 19 (0x6170000021c8): half _2_basic
Node 20 (0x6170000021e0): half _2_basic;
Node 21 (0x6170000021f8): _2_basic
Node 22 (0x617000002210): y
Node 23 (0x617000002228): 2.0
Node 24 (0x617000002240): (y * 2.0)
Node 25 (0x617000002258): (_2_basic = (y * 2.0))
Node 26 (0x617000002270): (_2_basic = (y * 2.0));
Node 27 (0x617000002288): sk_FragColor
Node 28 (0x6170000022a0): sk_FragColor.y
Node 29 (0x6170000022b8): _2_basic
Node 30 (0x6170000022d0): (sk_FragColor.y = _2_basic)
Node 31 (0x6170000022e8): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = (1.0 * 2.0)]
Entrances: [0]
Exits: []

about to simplify EX (1.0 * 2.0) 
optimized to 2.0 
coerced to 2.0 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002380): half _0_basic
Node 1 (0x617000002398): half _0_basic;
Node 2 (0x6170000023b0): 1.0
Node 3 (0x6170000023c8): half _1_x = 1.0
Node 4 (0x6170000023e0): half _1_x = 1.0;
Node 5 =================================================================
==77179==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000014010 at pc 0x00010ca62c05 bp 0x7ffee32b2ee0 sp 0x7ffee32b2ed8
READ of size 8 at 0x60d000014010 thread T0
    #0 0x10ca62c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10ca619a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x10ca60fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x10cb06d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x10cb15657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x10cb126e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x10c94d886 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000014010 is located 128 bytes inside of 136-byte region [0x60d000013f90,0x60d000014018)
freed by thread T0 here:
    #0 0x10e67fc6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10cdea8dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x10cc27e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10cb0208d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x10cb4d97d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x10cb01a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x10cafd4a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x10cb07012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x10cb15657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x10cb126e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x10c94d886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10e67f84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10cea5d8d in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable const*&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable const*&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x10ceaffec in SkSL::Inliner::inlineCall(SkSL::FunctionCall*, SkSL::SymbolTable*)::$_6::operator()(SkSL::String const&, SkSL::Type const*, SkSL::Modifiers, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*) const SkSLInliner.cpp:635
    #3 0x10ceaaf55 in SkSL::Inliner::inlineCall(SkSL::FunctionCall*, SkSL::SymbolTable*) SkSLInliner.cpp:678
    #4 0x10ccd192f in SkSL::IRGenerator::call(int, SkSL::FunctionDeclaration const&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2115
    #5 0x10ccd5503 in SkSL::IRGenerator::call(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2206
    #6 0x10ccf4521 in SkSL::IRGenerator::convertCallExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:2742
    #7 0x10ccbfeaa in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1318
    #8 0x10ccf1742 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1920
    #9 0x10ccbfc39 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #10 0x10cca6e58 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #11 0x10cc9a57b in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #12 0x10ccab0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #13 0x10cc9ba03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #14 0x10ccde8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #15 0x10cd43663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #16 0x10cb11e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #17 0x10c94d886 in main SkSLMain.cpp:242
    #18 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a000027b0: 00 00 00 00 00 fa fa fa fa fa fa fa fa fa 00 00
  0x1c1a000027c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa 00 00 00 00 00 00 00 00
  0x1c1a000027e0: 00 00 00 00 00 00 00 00 00 fa fa fa fa fa fa fa
  0x1c1a000027f0: fa fa fd fd fd fd fd fd fd fd fd fd fd fd fd fd
=>0x1c1a00002800: fd fd[fd]fa fa fa fa fa fa fa fa fa 00 00 00 00
  0x1c1a00002810: 00 00 00 00 00 00 00 00 00 00 00 00 00 fa fa fa
  0x1c1a00002820: fa fa fa fa fa fa 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002830: 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa fa
  0x1c1a00002840: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a00002850: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==77179==ABORTING
(0x6170000023f8): _0_basic
Node 6 (0x617000002410): 2.0
Node 7 (0x617000002428): (_0_basic = 2.0)
Node 8 (0x617000002440): (_0_basic = 2.0);
Node 9 (0x617000002458): sk_FragColor
Node 10 (0x617000002470): sk_FragColor.x
Node 11 (0x617000002488): _0_basic
Node 12 (0x6170000024a0): (sk_FragColor.x = _0_basic)
Node 13 (0x6170000024b8): (sk_FragColor.x = _0_basic);
Node 14 (0x6170000024d0): 2.0
Node 15 (0x6170000024e8): half y = 2.0
Node 16 (0x617000002500): half y = 2.0;
Node 17 (0x617000002518): half _2_basic
Node 18 (0x617000002530): half _2_basic;
Node 19 (0x617000002548): _2_basic
Node 20 (0x617000002560): y
Node 21 (0x617000002578): 2.0
Node 22 (0x617000002590): (y * 2.0)
Node 23 (0x6170000025a8): (_2_basic = (y * 2.0))
Node 24 (0x6170000025c0): (_2_basic = (y * 2.0));
Node 25 (0x6170000025d8): sk_FragColor
Node 26 (0x6170000025f0): sk_FragColor.y
Node 27 (0x617000002608): _2_basic
Node 28 (0x617000002620): (sk_FragColor.y = _2_basic)
Node 29 (0x617000002638): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = 2.0]
Entrances: [0]
Exits: []

about to simplify ST half _0_basic 
simplifying vardecl
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002380): half _0_basic
Node 1 (0x617000002398): half _0_basic;
Node 2 (0x6170000023b0): 1.0
Node 3 (0x6170000023c8): half _1_x = 1.0
Node 4 (0x6170000023e0): half _1_x = 1.0;
Node 5 (0x6170000023f8): _0_basic
Node 6 (0x617000002410): 2.0
Node 7 (0x617000002428): (_0_basic = 2.0)
Node 8 (0x617000002440): (_0_basic = 2.0);
Node 9 (0x617000002458): sk_FragColor
Node 10 (0x617000002470): sk_FragColor.x
Node 11 (0x617000002488): _0_basic
Node 12 (0x6170000024a0): (sk_FragColor.x = _0_basic)
Node 13 (0x6170000024b8): (sk_FragColor.x = _0_basic);
Node 14 (0x6170000024d0): 2.0
Node 15 (0x6170000024e8): half y = 2.0
Node 16 (0x617000002500): half y = 2.0;
Node 17 (0x617000002518): half _2_basic
Node 18 (0x617000002530): half _2_basic;
Node 19 (0x617000002548): _2_basic
Node 20 (0x617000002560): y
Node 21 (0x617000002578): 2.0
Node 22 (0x617000002590): (y * 2.0)
Node 23 (0x6170000025a8): (_2_basic = (y * 2.0))
Node 24 (0x6170000025c0): (_2_basic = (y * 2.0));
Node 25 (0x6170000025d8): sk_FragColor
Node 26 (0x6170000025f0): sk_FragColor.y
Node 27 (0x617000002608): _2_basic
Node 28 (0x617000002620): (sk_FragColor.y = _2_basic)
Node 29 (0x617000002638): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = 2.0]
Entrances: [0]
Exits: []

about to simplify ST half _0_basic; 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002380): half _0_basic
Node 1 (0x617000002398): half _0_basic;
Node 2 (0x6170000023b0): 1.0
Node 3 (0x6170000023c8): half _1_x = 1.0
Node 4 (0x6170000023e0): half _1_x = 1.0;
Node 5 (0x6170000023f8): _0_basic
Node 6 (0x617000002410): 2.0
Node 7 (0x617000002428): (_0_basic = 2.0)
Node 8 (0x617000002440): (_0_basic = 2.0);
Node 9 (0x617000002458): sk_FragColor
Node 10 (0x617000002470): sk_FragColor.x
Node 11 (0x617000002488): _0_basic
Node 12 (0x6170000024a0): (sk_FragColor.x = _0_basic)
Node 13 (0x6170000024b8): (sk_FragColor.x = _0_basic);
Node 14 (0x6170000024d0): 2.0
Node 15 (0x6170000024e8): half y = 2.0
Node 16 (0x617000002500): half y = 2.0;
Node 17 (0x617000002518): half _2_basic
Node 18 (0x617000002530): half _2_basic;
Node 19 (0x617000002548): _2_basic
Node 20 (0x617000002560): y
Node 21 (0x617000002578): 2.0
Node 22 (0x617000002590): (y * 2.0)
Node 23 (0x6170000025a8): (_2_basic = (y * 2.0))
Node 24 (0x6170000025c0): (_2_basic = (y * 2.0));
Node 25 (0x6170000025d8): sk_FragColor
Node 26 (0x6170000025f0): sk_FragColor.y
Node 27 (0x617000002608): _2_basic
Node 28 (0x617000002620): (sk_FragColor.y = _2_basic)
Node 29 (0x617000002638): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = 2.0]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002380): half _0_basic
Node 1 (0x617000002398): half _0_basic;
Node 2 (0x6170000023b0): 1.0
Node 3 (0x6170000023c8): half _1_x = 1.0
Node 4 (0x6170000023e0): half _1_x = 1.0;
Node 5 (0x6170000023f8): _0_basic
Node 6 (0x617000002410): 2.0
Node 7 (0x617000002428): (_0_basic = 2.0)
Node 8 (0x617000002440): (_0_basic = 2.0);
Node 9 (0x617000002458): sk_FragColor
Node 10 (0x617000002470): sk_FragColor.x
Node 11 (0x617000002488): _0_basic
Node 12 (0x6170000024a0): (sk_FragColor.x = _0_basic)
Node 13 (0x6170000024b8): (sk_FragColor.x = _0_basic);
Node 14 (0x6170000024d0): 2.0
Node 15 (0x6170000024e8): half y = 2.0
Node 16 (0x617000002500): half y = 2.0;
Node 17 (0x617000002518): half _2_basic
Node 18 (0x617000002530): half _2_basic;
Node 19 (0x617000002548): _2_basic
Node 20 (0x617000002560): y
Node 21 (0x617000002578): 2.0
Node 22 (0x617000002590): (y * 2.0)
Node 23 (0x6170000025a8): (_2_basic = (y * 2.0))
Node 24 (0x6170000025c0): (_2_basic = (y * 2.0));
Node 25 (0x6170000025d8): sk_FragColor
Node 26 (0x6170000025f0): sk_FragColor.y
Node 27 (0x617000002608): _2_basic
Node 28 (0x617000002620): (sk_FragColor.y = _2_basic)
Node 29 (0x617000002638): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0, half _1_x = 1.0, half _0_basic = 2.0]
Entrances: [0]
Exits: []

about to simplify ST half _1_x = 1.0 
simplifying vardecl
Block 0
-------
Before: [half _2_basic = <undefined>, half y = <undefined>, half _1_x = <undefined>, half _0_basic = <undefined>]
Entrances: []
Node 0 (0x617000002380): half _0_basic
Node 1 (0x617000002398): half _0_basic;
Node 2 (0x6170000023b0): ;
Node 3 (0x6170000023c8): half ;
Node 4 (0x6170000023e0): _0_basic
Node 5 (0x6170000023f8): 2.0
Node 6 (0x617000002410): (_0_basic = 2.0)
Node 7 (0x617000002428): (_0_basic = 2.0);
Node 8 (0x617000002440): sk_FragColor
Node 9 (0x617000002458): sk_FragColor.x
Node 10 (0x617000002470): _0_basic
Node 11 (0x617000002488): (sk_FragColor.x = _0_basic)
Node 12 (0x6170000024a0): (sk_FragColor.x = _0_basic);
Node 13 (0x6170000024b8): 2.0
Node 14 (0x6170000024d0): half y = 2.0
Node 15 (0x6170000024e8): half y = 2.0;
Node 16 (0x617000002500): half _2_basic
Node 17 (0x617000002518): half _2_basic;
Node 18 (0x617000002530): _2_basic
Node 19 (0x617000002548): y
Node 20 (0x617000002560): 2.0
Node 21 (0x617000002578): (y * 2.0)
Node 22 (0x617000002590): (_2_basic = (y * 2.0))
Node 23 (0x6170000025a8): (_2_basic = (y * 2.0));
Node 24 (0x6170000025c0): sk_FragColor
Node 25 (0x6170000025d8): sk_FragColor.y
Node 26 (0x6170000025f0): _2_basic
Node 27 (0x617000002608): (sk_FragColor.y = _2_basic)
Node 28 (0x617000002620): (sk_FragColor.y = _2_basic);
Exits: [1]

Block 1
-------
Before: [half _2_basic = (y * 2.0), half y = 2.0
