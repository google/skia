### Compilation failed:

Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2(1.0) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2(2.0) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2[2](float2(1.0), float2(2.0)) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify ST float2[2] x[2] = float2[2](float2(1.0), float2(2.0)) 
simplifying vardecl
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify ST float2 x = float2[2](float2(1.0), float2(2.0)); 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX 3.0 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2(3.0) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX 4.0 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2(4.0) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2[2](float2(3.0), float2(4.0)) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify ST float2[2] y = float2[2](float2(3.0), float2(4.0)) 
simplifying vardecl
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify ST float2[2] y = float2[2](float2(3.0), float2(4.0)); 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): x
Node 16 (0x617000002180): 0
Node 17 (0x617000002198): x[0]
Node 18 (0x6170000021b0): half2(x[0])
Node 19 (0x6170000021c8): y
Node 20 (0x6170000021e0): 1
Node 21 (0x6170000021f8): y[1]
Node 22 (0x617000002210): half2(y[1])
Node 23 (0x617000002228): half4(half2(x[0]), half2(y[1]))
Node 24 (0x617000002240): (sk_FragColor = half4(half2(x[0]), half2(y[1])))
Node 25 (0x617000002258): (sk_FragColor = half4(half2(x[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to float2[2](float2(1.0), float2(2.0)) 
coerced to float2[2](float2(1.0), float2(2.0)) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): float2(1.0)
Node 17 (0x617000002198): 2.0
Node 18 (0x6170000021b0): float2(2.0)
Node 19 (0x6170000021c8): float2[2](float2(1.0), float2(2.0))
Node 20 (0x6170000021e0): 0
Node 21 (0x6170000021f8): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x617000002210): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x617000002228): y
Node 24 (0x617000002240): 1
Node 25 (0x617000002258): y[1]
Node 26 (0x617000002270): half2(y[1])
Node 27 (0x617000002288): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1]))
Node 28 (0x6170000022a0): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1])))
Node 29 (0x6170000022b8): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): float2(1.0)
Node 17 (0x617000002198): 2.0
Node 18 (0x6170000021b0): float2(2.0)
Node 19 (0x6170000021c8): float2[2](float2(1.0), float2(2.0))
Node 20 (0x6170000021e0): 0
Node 21 (0x6170000021f8): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x617000002210): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x617000002228): y
Node 24 (0x617000002240): 1
Node 25 (0x617000002258): y[1]
Node 26 (0x617000002270): half2(y[1])
Node 27 (0x617000002288): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1]))
Node 28 (0x6170000022a0): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1])))
Node 29 (0x6170000022b8): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2[2](float2(1.0), float2(2.0))[0] 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): float2(1.0)
Node 17 (0x617000002198): 2.0
Node 18 (0x6170000021b0): float2(2.0)
Node 19 (0x6170000021c8): float2[2](float2(1.0), float2(2.0))
Node 20 (0x6170000021e0): 0
Node 21 (0x6170000021f8): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x617000002210): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x617000002228): y
Node 24 (0x617000002240): 1
Node 25 (0x617000002258): y[1]
Node 26 (0x617000002270): half2(y[1])
Node 27 (0x617000002288): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1]))
Node 28 (0x6170000022a0): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1])))
Node 29 (0x6170000022b8): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX half2(float2[2](float2(1.0), float2(2.0))[0]) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float2(1.0)
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float2(2.0)
Node 4 (0x617000002060): float2[2](float2(1.0), float2(2.0))
Node 5 (0x617000002078): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x617000002090): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x6170000020a8): 3.0
Node 8 (0x6170000020c0): float2(3.0)
Node 9 (0x6170000020d8): 4.0
Node 10 (0x6170000020f0): float2(4.0)
Node 11 (0x617000002108): float2[2](float2(3.0), float2(4.0))
Node 12 (0x617000002120): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x617000002138): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x617000002150): sk_FragColor
Node 15 (0x617000002168): 1.0
Node 16 (0x617000002180): float2(1.0)
Node 17 (0x617000002198): 2.0
Node 18 (0x6170000021b0): float2(2.0)
Node 19 (0x6170000021c8): float2[2](float2(1.0), float2(2.0))
Node 20 (0x6170000021e0): 0
Node 21 (0x6170000021f8): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x617000002210): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x617000002228): y
Node 24 (0x617000002240): 1
Node 25 (0x617000002258): y[1]
Node 26 (0x617000002270): half2(y[1])
Node 27 (0x617000002288): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1]))
Node 28 (0x6170000022a0): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1])))
Node 29 (0x6170000022b8): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(y[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX y 
optimized to float2[2](float2(3.0), float2(4.0)) 
coerced to float2[2](float2(3.0), float2(4.0)) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2[2](float2(3.0), float2(4.0))[1] 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX half2(float2[2](float2(3.0), float2(4.0))[1]) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))); 
simplifying expr
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2=================================================================
==77419==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013cd0 at pc 0x000107d04c05 bp 0x7ffee8010f00 sp 0x7ffee8010ef8
READ of size 8 at 0x60d000013cd0 thread T0
    #0 0x107d04c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x107d039a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x107d02fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x107da8d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x107db7657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x107db46e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x107bf0108 in main SkSLMain.cpp:258
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013cd0 is located 128 bytes inside of 136-byte region [0x60d000013c50,0x60d000013cd8)
freed by thread T0 here:
    #0 0x10991dc6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10808c8dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x107ec9e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x107da408d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x107def97d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x107da3a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x107d9f4a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x107da9012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x107db7657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x107db46e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x107bf0108 in main SkSLMain.cpp:258
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10991d84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x107f64fdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x107f579a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x107f3e116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x107f3c17e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x107f4d0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x107f3da03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x107f808cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x107fe5663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x107db3e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x107bf0108 in main SkSLMain.cpp:258
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a00002740: 00 00 00 00 00 00 00 00 00 00 00 00 00 fa fa fa
  0x1c1a00002750: fa fa fa fa fa fa 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002760: 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa fa
  0x1c1a00002770: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002780: 00 fa fa fa fa fa fa fa fa fa fd fd fd fd fd fd
=>0x1c1a00002790: fd fd fd fd fd fd fd fd fd fd[fd]fa fa fa fa fa
  0x1c1a000027a0: fa fa fa fa 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a000027b0: 00 00 00 00 00 fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027c0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027e0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==77419==ABORTING
(1.0) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2(2.0) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify EX float2[2](float2(1.0), float2(2.0)) 
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2(1.0)
Node 2 (0x61b0000023b0): 2.0
Node 3 (0x61b0000023c8): float2(2.0)
Node 4 (0x61b0000023e0): float2[2](float2(1.0), float2(2.0))
Node 5 (0x61b0000023f8): float2[2] x[2] = float2[2](float2(1.0), float2(2.0))
Node 6 (0x61b000002410): float2 x = float2[2](float2(1.0), float2(2.0));
Node 7 (0x61b000002428): 3.0
Node 8 (0x61b000002440): float2(3.0)
Node 9 (0x61b000002458): 4.0
Node 10 (0x61b000002470): float2(4.0)
Node 11 (0x61b000002488): float2[2](float2(3.0), float2(4.0))
Node 12 (0x61b0000024a0): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 13 (0x61b0000024b8): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): float2(1.0)
Node 17 (0x61b000002518): 2.0
Node 18 (0x61b000002530): float2(2.0)
Node 19 (0x61b000002548): float2[2](float2(1.0), float2(2.0))
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float2[2](float2(1.0), float2(2.0))[0]
Node 22 (0x61b000002590): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 23 (0x61b0000025a8): 3.0
Node 24 (0x61b0000025c0): float2(3.0)
Node 25 (0x61b0000025d8): 4.0
Node 26 (0x61b0000025f0): float2(4.0)
Node 27 (0x61b000002608): float2[2](float2(3.0), float2(4.0))
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float2[2](float2(3.0), float2(4.0))[1]
Node 30 (0x61b000002650): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 31 (0x61b000002668): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 32 (0x61b000002680): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 33 (0x61b000002698): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0)), float2[2] x = float2[2](float2(1.0), float2(2.0))]
Entrances: [0]
Exits: []

about to simplify ST float2[2] x[2] = float2[2](float2(1.0), float2(2.0)) 
simplifying vardecl
Block 0
-------
Before: [float2[2] y = <undefined>, float2[2] x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): ;
Node 1 (0x61b000002398): float2 ;
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): float2(3.0)
Node 4 (0x61b0000023e0): 4.0
Node 5 (0x61b0000023f8): float2(4.0)
Node 6 (0x61b000002410): float2[2](float2(3.0), float2(4.0))
Node 7 (0x61b000002428): float2[2] y = float2[2](float2(3.0), float2(4.0))
Node 8 (0x61b000002440): float2[2] y = float2[2](float2(3.0), float2(4.0));
Node 9 (0x61b000002458): sk_FragColor
Node 10 (0x61b000002470): 1.0
Node 11 (0x61b000002488): float2(1.0)
Node 12 (0x61b0000024a0): 2.0
Node 13 (0x61b0000024b8): float2(2.0)
Node 14 (0x61b0000024d0): float2[2](float2(1.0), float2(2.0))
Node 15 (0x61b0000024e8): 0
Node 16 (0x61b000002500): float2[2](float2(1.0), float2(2.0))[0]
Node 17 (0x61b000002518): half2(float2[2](float2(1.0), float2(2.0))[0])
Node 18 (0x61b000002530): 3.0
Node 19 (0x61b000002548): float2(3.0)
Node 20 (0x61b000002560): 4.0
Node 21 (0x61b000002578): float2(4.0)
Node 22 (0x61b000002590): float2[2](float2(3.0), float2(4.0))
Node 23 (0x61b0000025a8): 1
Node 24 (0x61b0000025c0): float2[2](float2(3.0), float2(4.0))[1]
Node 25 (0x61b0000025d8): half2(float2[2](float2(3.0), float2(4.0))[1])
Node 26 (0x61b0000025f0): half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1]))
Node 27 (0x61b000002608): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])))
Node 28 (0x61b000002620): (sk_FragColor = half4(half2(float2[2](float2(1.0), float2(2.0))[0]), half2(float2[2](float2(3.0), float2(4.0))[1])));
Exits: [1]

Block 1
-------
Before: [float2[2] y = float2[2](float2(3.0), float2(4.0))
