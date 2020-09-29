### Compilation failed:

Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color.xy
Node 2 (0x613000001ab0): color
Node 3 (0x613000001ac8): color.yx
Node 4 (0x613000001ae0): (color.xy = color.yx)
Node 5 (0x613000001af8): (color.xy = color.yx);
Node 6 (0x613000001b10): x
Node 7 (0x613000001b28): y
Node 8 (0x613000001b40): (x + y)
Node 9 (0x613000001b58): z
Node 10 (0x613000001b70): half2((x + y), z)
Node 11 (0x613000001b88): return half2((x + y), z);
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

about to simplify EX color 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color.xy
Node 2 (0x613000001ab0): color
Node 3 (0x613000001ac8): color.yx
Node 4 (0x613000001ae0): (color.xy = color.yx)
Node 5 (0x613000001af8): (color.xy = color.yx);
Node 6 (0x613000001b10): x
Node 7 (0x613000001b28): y
Node 8 (0x613000001b40): (x + y)
Node 9 (0x613000001b58): z
Node 10 (0x613000001b70): half2((x + y), z)
Node 11 (0x613000001b88): return half2((x + y), z);
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

about to simplify EX color.xy 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX color 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX color.yx 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX (color = color.yx) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify ST (color = color.yx); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX y 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX (x + y) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX z 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX half2((x + y), z) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify ST return half2((x + y), z); 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX color 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX color 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX color.yx 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX (color = color.yx) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify ST (color = color.yx); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX y 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX (x + y) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX z 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify EX half2((x + y), z) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x613000001a80): color
Node 1 (0x613000001a98): color
Node 2 (0x613000001ab0): color.yx
Node 3 (0x613000001ac8): (color = color.yx)
Node 4 (0x613000001ae0): (color = color.yx);
Node 5 (0x613000001af8): x
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): (x + y)
Node 8 (0x613000001b40): z
Node 9 (0x613000001b58): half2((x + y), z)
Node 10 (0x613000001b70): return half2((x + y), z);
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

about to simplify ST return half2((x + y), z); 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half2 _0_tricky 
simplifying vardecl
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half2 _0_tricky; 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half _1_x = 1.0 
simplifying vardecl
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half _1_x = 1.0; 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half _2_y = 2.0 
simplifying vardecl
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half _2_y = 2.0; 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX color 
optimizing varref 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX color.xz 
optimizing swiz 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half2 _3_color = color.xz 
simplifying vardecl
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half2 _3_color = color.xz; 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX 5.0 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half _4_z = 5.0 
simplifying vardecl
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half _4_z = 5.0; 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX _3_color 
optimizing varref 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color.xy
Node 17 (0x61b000002518): _3_color
Node 18 (0x61b000002530): _3_color.yx
Node 19 (0x61b000002548): (_3_color.xy = _3_color.yx)
Node 20 (0x61b000002560): (_3_color.xy = _3_color.yx);
Node 21 (0x61b000002578): _0_tricky
Node 22 (0x61b000002590): _1_x
Node 23 (0x61b0000025a8): _2_y
Node 24 (0x61b0000025c0): (_1_x + _2_y)
Node 25 (0x61b0000025d8): _4_z
Node 26 (0x61b0000025f0): half2((_1_x + _2_y), _4_z)
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 28 (0x61b000002620): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 29 (0x61b000002638): color
Node 30 (0x61b000002650): color.xz
Node 31 (0x61b000002668): _3_color
Node 32 (0x61b000002680): (color.xz = _3_color)
Node 33 (0x61b000002698): (color.xz = _3_color);
Node 34 (0x61b0000026b0): _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky
Node 36 (0x61b0000026e0): half2 t = _0_tricky;
Node 37 (0x61b0000026f8): color
Node 38 (0x61b000002710): color.yw
Node 39 (0x61b000002728): t
Node 40 (0x61b000002740): (color.yw = t)
Node 41 (0x61b000002758): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX _3_color.xy 
optimizing swiz 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color
Node 17 (0x61b000002518): _3_color.yx
Node 18 (0x61b000002530): (_3_color = _3_color.yx)
Node 19 (0x61b000002548): (_3_color = _3_color.yx);
Node 20 (0x61b000002560): _0_tricky
Node 21 (0x61b000002578): _1_x
Node 22 (0x61b000002590): _2_y
Node 23 (0x61b0000025a8): (_1_x + _2_y)
Node 24 (0x61b0000025c0): _4_z
Node 25 (0x61b0000025d8): half2((_1_x + _2_y), _4_z)
Node 26 (0x61b0000025f0): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 28 (0x61b000002620): color
Node 29 (0x61b000002638): color.xz
Node 30 (0x61b000002650): _3_color
Node 31 (0x61b000002668): (color.xz = _3_color)
Node 32 (0x61b000002680): (color.xz = _3_color);
Node 33 (0x61b000002698): _0_tricky
Node 34 (0x61b0000026b0): half2 t = _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky;
Node 36 (0x61b0000026e0): color
Node 37 (0x61b0000026f8): color.yw
Node 38 (0x61b000002710): t
Node 39 (0x61b000002728): (color.yw = t)
Node 40 (0x61b000002740): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX _3_color 
optimizing varref 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color
Node 17 (0x61b000002518): _3_color.yx
Node 18 (0x61b000002530): (_3_color = _3_color.yx)
Node 19 (0x61b000002548): (_3_color = _3_color.yx);
Node 20 (0x61b000002560): _0_tricky
Node 21 (0x61b000002578): _1_x
Node 22 (0x61b000002590): _2_y
Node 23 (0x61b0000025a8): (_1_x + _2_y)
Node 24 (0x61b0000025c0): _4_z
Node 25 (0x61b0000025d8): half2((_1_x + _2_y), _4_z)
Node 26 (0x61b0000025f0): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 28 (0x61b000002620): color
Node 29 (0x61b000002638): color.xz
Node 30 (0x61b000002650): _3_color
Node 31 (0x61b000002668): (color.xz = _3_color)
Node 32 (0x61b000002680): (color.xz = _3_color);
Node 33 (0x61b000002698): _0_tricky
Node 34 (0x61b0000026b0): half2 t = _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky;
Node 36 (0x61b0000026e0): color
Node 37 (0x61b0000026f8): color.yw
Node 38 (0x61b000002710): t
Node 39 (0x61b000002728): (color.yw = t)
Node 40 (0x61b000002740): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX _3_color.yx 
optimizing swiz 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color
Node 17 (0x61b000002518): _3_color.yx
Node 18 (0x61b000002530): (_3_color = _3_color.yx)
Node 19 (0x61b000002548): (_3_color = _3_color.yx);
Node 20 (0x61b000002560): _0_tricky
Node 21 (0x61b000002578): _1_x
Node 22 (0x61b000002590): _2_y
Node 23 (0x61b0000025a8): (_1_x + _2_y)
Node 24 (0x61b0000025c0): _4_z
Node 25 (0x61b0000025d8): half2((_1_x + _2_y), _4_z)
Node 26 (0x61b0000025f0): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 28 (0x61b000002620): color
Node 29 (0x61b000002638): color.xz
Node 30 (0x61b000002650): _3_color
Node 31 (0x61b000002668): (color.xz = _3_color)
Node 32 (0x61b000002680): (color.xz = _3_color);
Node 33 (0x61b000002698): _0_tricky
Node 34 (0x61b0000026b0): half2 t = _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky;
Node 36 (0x61b0000026e0): color
Node 37 (0x61b0000026f8): color.yw
Node 38 (0x61b000002710): t
Node 39 (0x61b000002728): (color.yw = t)
Node 40 (0x61b000002740): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX (_3_color = _3_color.yx) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color
Node 17 (0x61b000002518): _3_color.yx
Node 18 (0x61b000002530): (_3_color = _3_color.yx)
Node 19 (0x61b000002548): (_3_color = _3_color.yx);
Node 20 (0x61b000002560): _0_tricky
Node 21 (0x61b000002578): _1_x
Node 22 (0x61b000002590): _2_y
Node 23 (0x61b0000025a8): (_1_x + _2_y)
Node 24 (0x61b0000025c0): _4_z
Node 25 (0x61b0000025d8): half2((_1_x + _2_y), _4_z)
Node 26 (0x61b0000025f0): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 28 (0x61b000002620): color
Node 29 (0x61b000002638): color.xz
Node 30 (0x61b000002650): _3_color
Node 31 (0x61b000002668): (color.xz = _3_color)
Node 32 (0x61b000002680): (color.xz = _3_color);
Node 33 (0x61b000002698): _0_tricky
Node 34 (0x61b0000026b0): half2 t = _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky;
Node 36 (0x61b0000026e0): color
Node 37 (0x61b0000026f8): color.yw
Node 38 (0x61b000002710): t
Node 39 (0x61b000002728): (color.yw = t)
Node 40 (0x61b000002740): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify ST (_3_color = _3_color.yx); 
simplifying expr
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color
Node 17 (0x61b000002518): _3_color.yx
Node 18 (0x61b000002530): (_3_color = _3_color.yx)
Node 19 (0x61b000002548): (_3_color = _3_color.yx);
Node 20 (0x61b000002560): _0_tricky
Node 21 (0x61b000002578): _1_x
Node 22 (0x61b000002590): _2_y
Node 23 (0x61b0000025a8): (_1_x + _2_y)
Node 24 (0x61b0000025c0): _4_z
Node 25 (0x61b0000025d8): half2((_1_x + _2_y), _4_z)
Node 26 (0x61b0000025f0): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 28 (0x61b000002620): color
Node 29 (0x61b000002638): color.xz
Node 30 (0x61b000002650): _3_color
Node 31 (0x61b000002668): (color.xz = _3_color)
Node 32 (0x61b000002680): (color.xz = _3_color);
Node 33 (0x61b000002698): _0_tricky
Node 34 (0x61b0000026b0): half2 t = _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky;
Node 36 (0x61b0000026e0): color
Node 37 (0x61b0000026f8): color.yw
Node 38 (0x61b000002710): t
Node 39 (0x61b000002728): (color.yw = t)
Node 40 (0x61b000002740): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX _0_tricky 
optimizing varref 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color
Node 17 (0x61b000002518): _3_color.yx
Node 18 (0x61b000002530): (_3_color = _3_color.yx)
Node 19 (0x61b000002548): (_3_color = _3_color.yx);
Node 20 (0x61b000002560): _0_tricky
Node 21 (0x61b000002578): _1_x
Node 22 (0x61b000002590): _2_y
Node 23 (0x61b0000025a8): (_1_x + _2_y)
Node 24 (0x61b0000025c0): _4_z
Node 25 (0x61b0000025d8): half2((_1_x + _2_y), _4_z)
Node 26 (0x61b0000025f0): (_0_tricky = half2((_1_x + _2_y), _4_z))
Node 27 (0x61b000002608): (_0_tricky = half2((_1_x + _2_y), _4_z));
Node 28 (0x61b000002620): color
Node 29 (0x61b000002638): color.xz
Node 30 (0x61b000002650): _3_color
Node 31 (0x61b000002668): (color.xz = _3_color)
Node 32 (0x61b000002680): (color.xz = _3_color);
Node 33 (0x61b000002698): _0_tricky
Node 34 (0x61b0000026b0): half2 t = _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky;
Node 36 (0x61b0000026e0): color
Node 37 (0x61b0000026f8): color.yw
Node 38 (0x61b000002710): t
Node 39 (0x61b000002728): (color.yw = t)
Node 40 (0x61b000002740): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((_1_x + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX _1_x 
optimized to 1.0 
coerced to 1.0 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color
Node 17 (0x61b000002518): _3_color.yx
Node 18 (0x61b000002530): (_3_color = _3_color.yx)
Node 19 (0x61b000002548): (_3_color = _3_color.yx);
Node 20 (0x61b000002560): _0_tricky
Node 21 (0x61b000002578): 1.0
Node 22 (0x61b000002590): _2_y
Node 23 (0x61b0000025a8): (1.0 + _2_y)
Node 24 (0x61b0000025c0): _4_z
Node 25 (0x61b0000025d8): half2((1.0 + _2_y), _4_z)
Node 26 (0x61b0000025f0): (_0_tricky = half2((1.0 + _2_y), _4_z))
Node 27 (0x61b000002608): (_0_tricky = half2((1.0 + _2_y), _4_z));
Node 28 (0x61b000002620): color
Node 29 (0x61b000002638): color.xz
Node 30 (0x61b000002650): _3_color
Node 31 (0x61b000002668): (color.xz = _3_color)
Node 32 (0x61b000002680): (color.xz = _3_color);
Node 33 (0x61b000002698): _0_tricky
Node 34 (0x61b0000026b0): half2 t = _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky;
Node 36 (0x61b0000026e0): color
Node 37 (0x61b0000026f8): color.yw
Node 38 (0x61b000002710): t
Node 39 (0x61b000002728): (color.yw = t)
Node 40 (0x61b000002740): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky =================================================================
==77738==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013f40 at pc 0x00010ed53c05 bp 0x7ffee0fc1f00 sp 0x7ffee0fc1ef8
READ of size 8 at 0x60d000013f40 thread T0
    #0 0x10ed53c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10ed529a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x10ed51fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x10edf7d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x10ee06657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x10ee036e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x10ec3e886 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013f40 is located 128 bytes inside of 136-byte region [0x60d000013ec0,0x60d000013f48)
freed by thread T0 here:
    #0 0x11096fc6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10f0db8dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x10ef18e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10edf308d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x10ee3e97d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x10edf2a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x10edee4a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x10edf8012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x10ee06657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x10ee036e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x10ec3e886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x11096f84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10f196d8d in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable const*&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable const*&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x10f1a0fec in SkSL::Inliner::inlineCall(SkSL::FunctionCall*, SkSL::SymbolTable*)::$_6::operator()(SkSL::String const&, SkSL::Type const*, SkSL::Modifiers, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*) const SkSLInliner.cpp:635
    #3 0x10f19bf55 in SkSL::Inliner::inlineCall(SkSL::FunctionCall*, SkSL::SymbolTable*) SkSLInliner.cpp:678
    #4 0x10efc292f in SkSL::IRGenerator::call(int, SkSL::FunctionDeclaration const&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2115
    #5 0x10efc6503 in SkSL::IRGenerator::call(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2206
    #6 0x10efe5521 in SkSL::IRGenerator::convertCallExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:2742
    #7 0x10efb0eaa in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1318
    #8 0x10efa5657 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:452
    #9 0x10ef8d116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #10 0x10ef8b17e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #11 0x10ef9c0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #12 0x10ef8ca03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #13 0x10efcf8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #14 0x10f034663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #15 0x10ee02e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #16 0x10ec3e886 in main SkSLMain.cpp:242
    #17 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a00002790: 00 00 00 00 00 00 00 00 00 00 00 fa fa fa fa fa
  0x1c1a000027a0: fa fa fa fa 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a000027b0: 00 00 00 00 00 fa fa fa fa fa fa fa fa fa 00 00
  0x1c1a000027c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
=>0x1c1a000027e0: fd fd fd fd fd fd fd fd[fd]fa fa fa fa fa fa fa
  0x1c1a000027f0: fa fa 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002800: 00 00 00 fa fa fa fa fa fa fa fa fa 00 00 00 00
  0x1c1a00002810: 00 00 00 00 00 00 00 00 00 00 00 00 00 fa fa fa
  0x1c1a00002820: fa fa fa fa fa fa 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002830: 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa fa
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
==77738==ABORTING
= half2((1.0 + _2_y), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX _2_y 
optimized to 2.0 
coerced to 2.0 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002380): half2 _0_tricky
Node 1 (0x61b000002398): half2 _0_tricky;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): half _1_x = 1.0
Node 4 (0x61b0000023e0): half _1_x = 1.0;
Node 5 (0x61b0000023f8): 2.0
Node 6 (0x61b000002410): half _2_y = 2.0
Node 7 (0x61b000002428): half _2_y = 2.0;
Node 8 (0x61b000002440): color
Node 9 (0x61b000002458): color.xz
Node 10 (0x61b000002470): half2 _3_color = color.xz
Node 11 (0x61b000002488): half2 _3_color = color.xz;
Node 12 (0x61b0000024a0): 5.0
Node 13 (0x61b0000024b8): half _4_z = 5.0
Node 14 (0x61b0000024d0): half _4_z = 5.0;
Node 15 (0x61b0000024e8): _3_color
Node 16 (0x61b000002500): _3_color
Node 17 (0x61b000002518): _3_color.yx
Node 18 (0x61b000002530): (_3_color = _3_color.yx)
Node 19 (0x61b000002548): (_3_color = _3_color.yx);
Node 20 (0x61b000002560): _0_tricky
Node 21 (0x61b000002578): 1.0
Node 22 (0x61b000002590): 2.0
Node 23 (0x61b0000025a8): (1.0 + 2.0)
Node 24 (0x61b0000025c0): _4_z
Node 25 (0x61b0000025d8): half2((1.0 + 2.0), _4_z)
Node 26 (0x61b0000025f0): (_0_tricky = half2((1.0 + 2.0), _4_z))
Node 27 (0x61b000002608): (_0_tricky = half2((1.0 + 2.0), _4_z));
Node 28 (0x61b000002620): color
Node 29 (0x61b000002638): color.xz
Node 30 (0x61b000002650): _3_color
Node 31 (0x61b000002668): (color.xz = _3_color)
Node 32 (0x61b000002680): (color.xz = _3_color);
Node 33 (0x61b000002698): _0_tricky
Node 34 (0x61b0000026b0): half2 t = _0_tricky
Node 35 (0x61b0000026c8): half2 t = _0_tricky;
Node 36 (0x61b0000026e0): color
Node 37 (0x61b0000026f8): color.yw
Node 38 (0x61b000002710): t
Node 39 (0x61b000002728): (color.yw = t)
Node 40 (0x61b000002740): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = <defined>, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2((1.0 + 2.0), _4_z)]
Entrances: [0]
Exits: []

about to simplify EX (1.0 + 2.0) 
optimized to 3.0 
coerced to 3.0 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): half2 _0_tricky
Node 1 (0x61b000002a98): half2 _0_tricky;
Node 2 (0x61b000002ab0): 1.0
Node 3 (0x61b000002ac8): half _1_x = 1.0
Node 4 (0x61b000002ae0): half _1_x = 1.0;
Node 5 (0x61b000002af8): 2.0
Node 6 (0x61b000002b10): half _2_y = 2.0
Node 7 (0x61b000002b28): half _2_y = 2.0;
Node 8 (0x61b000002b40): color
Node 9 (0x61b000002b58): color.xz
Node 10 (0x61b000002b70): half2 _3_color = color.xz
Node 11 (0x61b000002b88): half2 _3_color = color.xz;
Node 12 (0x61b000002ba0): 5.0
Node 13 (0x61b000002bb8): half _4_z = 5.0
Node 14 (0x61b000002bd0): half _4_z = 5.0;
Node 15 (0x61b000002be8): _3_color
Node 16 (0x61b000002c00): _3_color
Node 17 (0x61b000002c18): _3_color.yx
Node 18 (0x61b000002c30): (_3_color = _3_color.yx)
Node 19 (0x61b000002c48): (_3_color = _3_color.yx);
Node 20 (0x61b000002c60): _0_tricky
Node 21 (0x61b000002c78): 3.0
Node 22 (0x61b000002c90): _4_z
Node 23 (0x61b000002ca8): half2(3.0, _4_z)
Node 24 (0x61b000002cc0): (_0_tricky = half2(3.0, _4_z))
Node 25 (0x61b000002cd8): (_0_tricky = half2(3.0, _4_z));
Node 26 (0x61b000002cf0): color
Node 27 (0x61b000002d08): color.xz
Node 28 (0x61b000002d20): _3_color
Node 29 (0x61b000002d38): (color.xz = _3_color)
Node 30 (0x61b000002d50): (color.xz = _3_color);
Node 31 (0x61b000002d68): _0_tricky
Node 32 (0x61b000002d80): half2 t = _0_tricky
Node 33 (0x61b000002d98): half2 t = _0_tricky;
Node 34 (0x61b000002db0): color
Node 35 (0x61b000002dc8): color.yw
Node 36 (0x61b000002de0): t
Node 37 (0x61b000002df8): (color.yw = t)
Node 38 (0x61b000002e10): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = _3_color.yx, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2(3.0, _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half2 _0_tricky 
simplifying vardecl
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): half2 _0_tricky
Node 1 (0x61b000002a98): half2 _0_tricky;
Node 2 (0x61b000002ab0): 1.0
Node 3 (0x61b000002ac8): half _1_x = 1.0
Node 4 (0x61b000002ae0): half _1_x = 1.0;
Node 5 (0x61b000002af8): 2.0
Node 6 (0x61b000002b10): half _2_y = 2.0
Node 7 (0x61b000002b28): half _2_y = 2.0;
Node 8 (0x61b000002b40): color
Node 9 (0x61b000002b58): color.xz
Node 10 (0x61b000002b70): half2 _3_color = color.xz
Node 11 (0x61b000002b88): half2 _3_color = color.xz;
Node 12 (0x61b000002ba0): 5.0
Node 13 (0x61b000002bb8): half _4_z = 5.0
Node 14 (0x61b000002bd0): half _4_z = 5.0;
Node 15 (0x61b000002be8): _3_color
Node 16 (0x61b000002c00): _3_color
Node 17 (0x61b000002c18): _3_color.yx
Node 18 (0x61b000002c30): (_3_color = _3_color.yx)
Node 19 (0x61b000002c48): (_3_color = _3_color.yx);
Node 20 (0x61b000002c60): _0_tricky
Node 21 (0x61b000002c78): 3.0
Node 22 (0x61b000002c90): _4_z
Node 23 (0x61b000002ca8): half2(3.0, _4_z)
Node 24 (0x61b000002cc0): (_0_tricky = half2(3.0, _4_z))
Node 25 (0x61b000002cd8): (_0_tricky = half2(3.0, _4_z));
Node 26 (0x61b000002cf0): color
Node 27 (0x61b000002d08): color.xz
Node 28 (0x61b000002d20): _3_color
Node 29 (0x61b000002d38): (color.xz = _3_color)
Node 30 (0x61b000002d50): (color.xz = _3_color);
Node 31 (0x61b000002d68): _0_tricky
Node 32 (0x61b000002d80): half2 t = _0_tricky
Node 33 (0x61b000002d98): half2 t = _0_tricky;
Node 34 (0x61b000002db0): color
Node 35 (0x61b000002dc8): color.yw
Node 36 (0x61b000002de0): t
Node 37 (0x61b000002df8): (color.yw = t)
Node 38 (0x61b000002e10): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = _3_color.yx, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2(3.0, _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half2 _0_tricky; 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): half2 _0_tricky
Node 1 (0x61b000002a98): half2 _0_tricky;
Node 2 (0x61b000002ab0): 1.0
Node 3 (0x61b000002ac8): half _1_x = 1.0
Node 4 (0x61b000002ae0): half _1_x = 1.0;
Node 5 (0x61b000002af8): 2.0
Node 6 (0x61b000002b10): half _2_y = 2.0
Node 7 (0x61b000002b28): half _2_y = 2.0;
Node 8 (0x61b000002b40): color
Node 9 (0x61b000002b58): color.xz
Node 10 (0x61b000002b70): half2 _3_color = color.xz
Node 11 (0x61b000002b88): half2 _3_color = color.xz;
Node 12 (0x61b000002ba0): 5.0
Node 13 (0x61b000002bb8): half _4_z = 5.0
Node 14 (0x61b000002bd0): half _4_z = 5.0;
Node 15 (0x61b000002be8): _3_color
Node 16 (0x61b000002c00): _3_color
Node 17 (0x61b000002c18): _3_color.yx
Node 18 (0x61b000002c30): (_3_color = _3_color.yx)
Node 19 (0x61b000002c48): (_3_color = _3_color.yx);
Node 20 (0x61b000002c60): _0_tricky
Node 21 (0x61b000002c78): 3.0
Node 22 (0x61b000002c90): _4_z
Node 23 (0x61b000002ca8): half2(3.0, _4_z)
Node 24 (0x61b000002cc0): (_0_tricky = half2(3.0, _4_z))
Node 25 (0x61b000002cd8): (_0_tricky = half2(3.0, _4_z));
Node 26 (0x61b000002cf0): color
Node 27 (0x61b000002d08): color.xz
Node 28 (0x61b000002d20): _3_color
Node 29 (0x61b000002d38): (color.xz = _3_color)
Node 30 (0x61b000002d50): (color.xz = _3_color);
Node 31 (0x61b000002d68): _0_tricky
Node 32 (0x61b000002d80): half2 t = _0_tricky
Node 33 (0x61b000002d98): half2 t = _0_tricky;
Node 34 (0x61b000002db0): color
Node 35 (0x61b000002dc8): color.yw
Node 36 (0x61b000002de0): t
Node 37 (0x61b000002df8): (color.yw = t)
Node 38 (0x61b000002e10): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = _3_color.yx, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2(3.0, _4_z)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): half2 _0_tricky
Node 1 (0x61b000002a98): half2 _0_tricky;
Node 2 (0x61b000002ab0): 1.0
Node 3 (0x61b000002ac8): half _1_x = 1.0
Node 4 (0x61b000002ae0): half _1_x = 1.0;
Node 5 (0x61b000002af8): 2.0
Node 6 (0x61b000002b10): half _2_y = 2.0
Node 7 (0x61b000002b28): half _2_y = 2.0;
Node 8 (0x61b000002b40): color
Node 9 (0x61b000002b58): color.xz
Node 10 (0x61b000002b70): half2 _3_color = color.xz
Node 11 (0x61b000002b88): half2 _3_color = color.xz;
Node 12 (0x61b000002ba0): 5.0
Node 13 (0x61b000002bb8): half _4_z = 5.0
Node 14 (0x61b000002bd0): half _4_z = 5.0;
Node 15 (0x61b000002be8): _3_color
Node 16 (0x61b000002c00): _3_color
Node 17 (0x61b000002c18): _3_color.yx
Node 18 (0x61b000002c30): (_3_color = _3_color.yx)
Node 19 (0x61b000002c48): (_3_color = _3_color.yx);
Node 20 (0x61b000002c60): _0_tricky
Node 21 (0x61b000002c78): 3.0
Node 22 (0x61b000002c90): _4_z
Node 23 (0x61b000002ca8): half2(3.0, _4_z)
Node 24 (0x61b000002cc0): (_0_tricky = half2(3.0, _4_z))
Node 25 (0x61b000002cd8): (_0_tricky = half2(3.0, _4_z));
Node 26 (0x61b000002cf0): color
Node 27 (0x61b000002d08): color.xz
Node 28 (0x61b000002d20): _3_color
Node 29 (0x61b000002d38): (color.xz = _3_color)
Node 30 (0x61b000002d50): (color.xz = _3_color);
Node 31 (0x61b000002d68): _0_tricky
Node 32 (0x61b000002d80): half2 t = _0_tricky
Node 33 (0x61b000002d98): half2 t = _0_tricky;
Node 34 (0x61b000002db0): color
Node 35 (0x61b000002dc8): color.yw
Node 36 (0x61b000002de0): t
Node 37 (0x61b000002df8): (color.yw = t)
Node 38 (0x61b000002e10): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = _3_color.yx, half _2_y = 2.0, half _1_x = 1.0, half2 _0_tricky = half2(3.0, _4_z)]
Entrances: [0]
Exits: []

about to simplify ST half _1_x = 1.0 
simplifying vardecl
Block 0
-------
Before: [half2 t = <undefined>, half _4_z = <undefined>, half2 _3_color = <undefined>, half _2_y = <undefined>, half _1_x = <undefined>, half2 _0_tricky = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): half2 _0_tricky
Node 1 (0x61b000002a98): half2 _0_tricky;
Node 2 (0x61b000002ab0): ;
Node 3 (0x61b000002ac8): half ;
Node 4 (0x61b000002ae0): 2.0
Node 5 (0x61b000002af8): half _2_y = 2.0
Node 6 (0x61b000002b10): half _2_y = 2.0;
Node 7 (0x61b000002b28): color
Node 8 (0x61b000002b40): color.xz
Node 9 (0x61b000002b58): half2 _3_color = color.xz
Node 10 (0x61b000002b70): half2 _3_color = color.xz;
Node 11 (0x61b000002b88): 5.0
Node 12 (0x61b000002ba0): half _4_z = 5.0
Node 13 (0x61b000002bb8): half _4_z = 5.0;
Node 14 (0x61b000002bd0): _3_color
Node 15 (0x61b000002be8): _3_color
Node 16 (0x61b000002c00): _3_color.yx
Node 17 (0x61b000002c18): (_3_color = _3_color.yx)
Node 18 (0x61b000002c30): (_3_color = _3_color.yx);
Node 19 (0x61b000002c48): _0_tricky
Node 20 (0x61b000002c60): 3.0
Node 21 (0x61b000002c78): _4_z
Node 22 (0x61b000002c90): half2(3.0, _4_z)
Node 23 (0x61b000002ca8): (_0_tricky = half2(3.0, _4_z))
Node 24 (0x61b000002cc0): (_0_tricky = half2(3.0, _4_z));
Node 25 (0x61b000002cd8): color
Node 26 (0x61b000002cf0): color.xz
Node 27 (0x61b000002d08): _3_color
Node 28 (0x61b000002d20): (color.xz = _3_color)
Node 29 (0x61b000002d38): (color.xz = _3_color);
Node 30 (0x61b000002d50): _0_tricky
Node 31 (0x61b000002d68): half2 t = _0_tricky
Node 32 (0x61b000002d80): half2 t = _0_tricky;
Node 33 (0x61b000002d98): color
Node 34 (0x61b000002db0): color.yw
Node 35 (0x61b000002dc8): t
Node 36 (0x61b000002de0): (color.yw = t)
Node 37 (0x61b000002df8): (color.yw = t);
Exits: [1]

Block 1
-------
Before: [half2 t = _0_tricky, half _4_z = 5.0, half2 _3_color = _3_color.yx, half _2_y = 2.0
