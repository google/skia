### Compilation failed:

Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): 0
Node 2 (0x610000000d70): v[0]
Node 3 (0x610000000d88): v
Node 4 (0x610000000da0): 1
Node 5 (0x610000000db8): v[1]
Node 6 (0x610000000dd0): (v[0] * v[1])
Node 7 (0x610000000de8): return (v[0] * v[1]);
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

about to simplify EX v 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): 0
Node 2 (0x610000000d70): v[0]
Node 3 (0x610000000d88): v
Node 4 (0x610000000da0): 1
Node 5 (0x610000000db8): v[1]
Node 6 (0x610000000dd0): (v[0] * v[1])
Node 7 (0x610000000de8): return (v[0] * v[1]);
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

about to simplify EX 0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): 0
Node 2 (0x610000000d70): v[0]
Node 3 (0x610000000d88): v
Node 4 (0x610000000da0): 1
Node 5 (0x610000000db8): v[1]
Node 6 (0x610000000dd0): (v[0] * v[1])
Node 7 (0x610000000de8): return (v[0] * v[1]);
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

about to simplify EX v[0] 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): 0
Node 2 (0x610000000d70): v[0]
Node 3 (0x610000000d88): v
Node 4 (0x610000000da0): 1
Node 5 (0x610000000db8): v[1]
Node 6 (0x610000000dd0): (v[0] * v[1])
Node 7 (0x610000000de8): return (v[0] * v[1]);
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

about to simplify EX v 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): 0
Node 2 (0x610000000d70): v[0]
Node 3 (0x610000000d88): v
Node 4 (0x610000000da0): 1
Node 5 (0x610000000db8): v[1]
Node 6 (0x610000000dd0): (v[0] * v[1])
Node 7 (0x610000000de8): return (v[0] * v[1]);
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

about to simplify EX 1 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): 0
Node 2 (0x610000000d70): v[0]
Node 3 (0x610000000d88): v
Node 4 (0x610000000da0): 1
Node 5 (0x610000000db8): v[1]
Node 6 (0x610000000dd0): (v[0] * v[1])
Node 7 (0x610000000de8): return (v[0] * v[1]);
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

about to simplify EX v[1] 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): 0
Node 2 (0x610000000d70): v[0]
Node 3 (0x610000000d88): v
Node 4 (0x610000000da0): 1
Node 5 (0x610000000db8): v[1]
Node 6 (0x610000000dd0): (v[0] * v[1])
Node 7 (0x610000000de8): return (v[0] * v[1]);
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

about to simplify EX (v[0] * v[1]) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): 0
Node 2 (0x610000000d70): v[0]
Node 3 (0x610000000d88): v
Node 4 (0x610000000da0): 1
Node 5 (0x610000000db8): v[1]
Node 6 (0x610000000dd0): (v[0] * v[1])
Node 7 (0x610000000de8): return (v[0] * v[1]);
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

about to simplify ST return (v[0] * v[1]); 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST float[2] y[2] 
simplifying vardecl
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST float z 
simplifying vardecl
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST float y, z; 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX y 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX y[0] 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX (y[0] = x) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST (y[0] = x); 
simplifying expr
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX y 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX y[1] 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX (x * 2.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX (y[1] = (x * 2.0)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST (y[1] = (x * 2.0)); 
simplifying expr
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST float _0_foo 
simplifying vardecl
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST float _0_foo; 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX _0_foo 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX y 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX y[0] 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX y 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX y[1] 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX (y[0] * y[1]) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX (_0_foo = (y[0] * y[1])) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST (_0_foo = (y[0] * y[1])); 
simplifying expr
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX z 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX _0_foo 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX (z = _0_foo) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST (z = _0_foo); 
simplifying expr
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX z 
optimizing varref 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify EX (x = z) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _0_foo = <undefined>, float z = <undefined>, float[2] y = <undefined>]
Entrances: []
Node 0 (0x61b000002380): float[2] y[2]
Node 1 (0x61b000002398): float z
Node 2 (0x61b0000023b0): float y, z;
Node 3 (0x61b0000023c8): y
Node 4 (0x61b0000023e0): 0
Node 5 (0x61b0000023f8): y[0]
Node 6 (0x61b000002410): x
Node 7 (0x61b000002428): (y[0] = x)
Node 8 (0x61b000002440): (y[0] = x);
Node 9 (0x61b000002458): y
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): y[1]
Node 12 (0x61b0000024a0): x
Node 13 (0x61b0000024b8): 2.0
Node 14 (0x61b0000024d0): (x * 2.0)
Node 15 (0x61b0000024e8): (y[1] = (x * 2.0))
Node 16 (0x61b000002500): (y[1] = (x * 2.0));
Node 17 (0x61b000002518): float _0_foo
Node 18 (0x61b000002530): float _0_foo;
Node 19 (0x61b000002548): _0_foo
Node 20 (0x61b000002560): y
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): y[0]
Node 23 (0x61b0000025a8): y
Node 24 (0x61b0000025c0): 1
Node 25 (0x61b0000025d8): y[1]
Node 26 (0x61b0000025f0): (y[0] * y[1])
Node 27 (0x61b000002608): (_0_foo = (y[0] * y[1]))
Node 28 (0x61b000002620): (_0_foo = (y[0] * y[1]));
Node 29 (0x61b000002638): z
Node 30 (0x61b000002650): _0_foo
Node 31 (0x61b000002668): (z = _0_foo)
Node 32 (0x61b000002680): (z = _0_foo);
Node 33 (0x61b000002698): x
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): (x = z)
Node 36 (0x61b0000026e0): (x = z);
Exits: [1]

Block 1
-------
Before: [float _0_foo = (y[0] * y[1]), float z = _0_foo, float[2] y = <defined>]
Entrances: [0]
Exits: []

about to simplify ST (x = z); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 10.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float x = 10.0 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float x = 10.0; 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float[2] _1_y[2] 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _2_z 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _1_y, _2_z; 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[0] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): x
Node 10 (0x61b000002b70): (_1_y[0] = x)
Node 11 (0x61b000002b88): (_1_y[0] = x);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to 10.0 
coerced to 10.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): 10.0
Node 10 (0x61b000002b70): (_1_y[0] = 10.0)
Node 11 (0x61b000002b88): (_1_y[0] = 10.0);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_1_y[0] = 10.0) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): 10.0
Node 10 (0x61b000002b70): (_1_y[0] = 10.0)
Node 11 (0x61b000002b88): (_1_y[0] = 10.0);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (_1_y[0] = 10.0); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): 10.0
Node 10 (0x61b000002b70): (_1_y[0] = 10.0)
Node 11 (0x61b000002b88): (_1_y[0] = 10.0);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): 10.0
Node 10 (0x61b000002b70): (_1_y[0] = 10.0)
Node 11 (0x61b000002b88): (_1_y[0] = 10.0);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): 10.0
Node 10 (0x61b000002b70): (_1_y[0] = 10.0)
Node 11 (0x61b000002b88): (_1_y[0] = 10.0);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[1] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): 10.0
Node 10 (0x61b000002b70): (_1_y[0] = 10.0)
Node 11 (0x61b000002b88): (_1_y[0] = 10.0);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): x
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (x * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (x * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (x * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to 10.0 
coerced to 10.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): 10.0
Node 10 (0x61b000002b70): (_1_y[0] = 10.0)
Node 11 (0x61b000002b88): (_1_y[0] = 10.0);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): 10.0
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (10.0 * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (10.0 * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (10.0 * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): 10.0
Node 1 (0x61b000002a98): float x = 10.0
Node 2 (0x61b000002ab0): float x = 10.0;
Node 3 (0x61b000002ac8): float[2] _1_y[2]
Node 4 (0x61b000002ae0): float _2_z
Node 5 (0x61b000002af8): float _1_y, _2_z;
Node 6 (0x61b000002b10): _1_y
Node 7 (0x61b000002b28): 0
Node 8 (0x61b000002b40): _1_y[0]
Node 9 (0x61b000002b58): 10.0
Node 10 (0x61b000002b70): (_1_y[0] = 10.0)
Node 11 (0x61b000002b88): (_1_y[0] = 10.0);
Node 12 (0x61b000002ba0): _1_y
Node 13 (0x61b000002bb8): 1
Node 14 (0x61b000002bd0): _1_y[1]
Node 15 (0x61b000002be8): 10.0
Node 16 (0x61b000002c00): 2.0
Node 17 (0x61b000002c18): (10.0 * 2.0)
Node 18 (0x61b000002c30): (_1_y[1] = (10.0 * 2.0))
Node 19 (0x61b000002c48): (_1_y[1] = (10.0 * 2.0));
Node 20 (0x61b000002c60): float _3_0_foo
Node 21 (0x61b000002c78): float _3_0_foo;
Node 22 (0x61b000002c90): _3_0_foo
Node 23 (0x61b000002ca8): _1_y
Node 24 (0x61b000002cc0): 0
Node 25 (0x61b000002cd8): _1_y[0]
Node 26 (0x61b000002cf0): _1_y
Node 27 (0x61b000002d08): 1
Node 28 (0x61b000002d20): _1_y[1]
Node 29 (0x61b000002d38): (_1_y[0] * _1_y[1])
Node 30 (0x61b000002d50): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 31 (0x61b000002d68): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 32 (0x61b000002d80): _2_z
Node 33 (0x61b000002d98): _3_0_foo
Node 34 (0x61b000002db0): (_2_z = _3_0_foo)
Node 35 (0x61b000002dc8): (_2_z = _3_0_foo);
Node 36 (0x61b000002de0): x
Node 37 (0x61b000002df8): _2_z
Node 38 (0x61b000002e10): (x = _2_z)
Node 39 (0x61b000002e28): (x = _2_z);
Node 40 (0x61b000002e40): false
Node 41 (0x61b000002e58): false;
Node 42 (0x61b000002e70): sk_FragColor
Node 43 (0x61b000002e88): x
Node 44 (0x61b000002ea0): half(x)
Node 45 (0x61b000002eb8): half4(half(x))
Node 46 (0x61b000002ed0): (sk_FragColor = half4(half(x)))
Node 47 (0x61b000002ee8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (10.0 * 2.0) 
optimized to 20.0 
coerced to 20.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 10.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float x = 10.0 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float x = 10.0; 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float[2] _1_y[2] 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _2_z 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _1_y, _2_z; 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[0] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 10.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_1_y[0] = 10.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (_1_y[0] = 10.0); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[1] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 20.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_1_y[1] = 20.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (_1_y[1] = 20.0); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _3_0_foo 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _3_0_foo; 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _3_0_foo 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[0] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[1] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_1_y[0] * _1_y[1]) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_3_0_foo = (_1_y[0] * _1_y[1])) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (_3_0_foo = (_1_y[0] * _1_y[1])); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _2_z 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _3_0_foo 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_2_z = _3_0_foo) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (_2_z = _3_0_foo); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _2_z 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (x = _2_z) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (x = _2_z); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX false 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): false
Node 39 (0x61b000003528): false;
Node 40 (0x61b000003540): sk_FragColor
Node 41 (0x61b000003558): x
Node 42 (0x61b000003570): half(x)
Node 43 (0x61b000003588): half4(half(x))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)))
Node 45 (0x61b0000035b8): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST false; 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX half(x) 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX half4(half(x)) 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half(x))) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half(x))); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 10.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float x = 10.0 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float x = 10.0; 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float[2] _1_y[2] 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _2_z 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _1_y, _2_z; 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[0] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 10.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_1_y[0] = 10.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (_1_y[0] = 10.0); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[1] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 20.0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_1_y[1] = 20.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (_1_y[1] = 20.0); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _3_0_foo 
simplifying vardecl
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST float _3_0_foo; 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _3_0_foo 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[0] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _1_y[1] 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_1_y[0] * _1_y[1]) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_3_0_foo = (_1_y[0] * _1_y[1])) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (_3_0_foo = (_1_y[0] * _1_y[1])); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _2_z 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _3_0_foo 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (_2_z = _3_0_foo) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (_2_z = _3_0_foo); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX _2_z 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (x = _2_z) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (x = _2_z); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST ; 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0=================================================================
==77602==ERROR: AddressSanitizer: heap-use-after-free on address 0x61100002b2b1 at pc 0x000103d44ada bp 0x7ffeedbafb60 sp 0x7ffeedbaf320
READ of size 8 at 0x61100002b2b1 thread T0
    #0 0x103d44ad9 in wrap_memmove+0x169 (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x1dad9)
    #1 0x7fff6f361289 in std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::append(char const*, unsigned long)+0x85 (libc++.1.dylib:x86_64+0x38289)
    #2 0x1027f4a53 in SkSL::String::operator+(SkSL::StringFragment) const SkSLString.cpp:90
    #3 0x1022e898d in SkSL::Variable::description() const SkSLVariable.h:53
    #4 0x102164774 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:66
    #5 0x102163fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #6 0x102209d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #7 0x102218657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #8 0x1022156e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #9 0x102051108 in main SkSLMain.cpp:258
    #10 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x61100002b2b1 is located 113 bytes inside of 240-byte region [0x61100002b240,0x61100002b330)
freed by thread T0 here:
    #0 0x103d7cc6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x1022c776d in SkSL::Type::~Type() SkSLType.h:56
    #2 0x10227eb71 in std::__1::default_delete<SkSL::Symbol const>::operator()(SkSL::Symbol const*) const memory:2368
    #3 0x10227e7fd in std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >::reset(SkSL::Symbol const*) memory:2623
    #4 0x10227e5d5 in std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >::~unique_ptr() memory:2577
    #5 0x10227e571 in std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >::~unique_ptr() memory:2577
    #6 0x10227e517 in std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >::destroy(std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >*) memory:1936
    #7 0x10227e494 in void std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::__destroy<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >(std::__1::integral_constant<bool, true>, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >&, std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >*) memory:1798
    #8 0x10227e3fc in void std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::destroy<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >(std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >&, std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >*) memory:1635
    #9 0x10227e28f in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::__destruct_at_end(std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >*) vector:426
    #10 0x10227e060 in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::clear() vector:369
    #11 0x10227d8d8 in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::~__vector_base() vector:463
    #12 0x10227d65d in std::__1::vector<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::~vector() vector:555
    #13 0x102279401 in std::__1::vector<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::~vector() vector:550
    #14 0x102279280 in SkSL::SymbolTable::~SymbolTable() SkSLSymbolTable.h:25
    #15 0x102278f41 in SkSL::SymbolTable::~SymbolTable() SkSLSymbolTable.h:25
    #16 0x1024d285f in std::__1::default_delete<SkSL::SymbolTable>::operator()(SkSL::SymbolTable*) const memory:2368
    #17 0x1024d17c8 in std::__1::__shared_ptr_pointer<SkSL::SymbolTable*, std::__1::default_delete<SkSL::SymbolTable>, std::__1::allocator<SkSL::SymbolTable> >::__on_zero_shared() memory:3541
    #18 0x10205e946 in std::__1::__shared_count::__release_shared() memory:3445
    #19 0x10205e6a7 in std::__1::__shared_weak_count::__release_shared() memory:3487
    #20 0x102086f91 in std::__1::shared_ptr<SkSL::SymbolTable>::~shared_ptr() memory:4212
    #21 0x1020579d1 in std::__1::shared_ptr<SkSL::SymbolTable>::~shared_ptr() memory:4210
    #22 0x10225faf1 in SkSL::IRNode::BlockData::~BlockData() SkSLIRNode.h:67
    #23 0x10225dfe1 in SkSL::IRNode::BlockData::~BlockData() SkSLIRNode.h:67
    #24 0x10281808a in SkSL::IRNode::NodeData::cleanup() SkSLIRNode.h:172
    #25 0x102818111 in SkSL::IRNode::NodeData::~NodeData() SkSLIRNode.h:165
    #26 0x1028153d1 in SkSL::IRNode::NodeData::~NodeData() SkSLIRNode.h:164
    #27 0x1028152b2 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #28 0x102251744 in SkSL::Statement::~Statement() SkSLStatement.h:19
    #29 0x10225fb54 in SkSL::Block::~Block() SkSLBlock.h:19

previously allocated by thread T0 here:
    #0 0x103d7c84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x1023c4150 in std::__1::__unique_if<SkSL::Type>::__unique_single std::__1::make_unique<SkSL::Type, SkSL::String&, SkSL::Type::TypeKind, SkSL::Type const&, int>(SkSL::String&, SkSL::Type::TypeKind&&, SkSL::Type const&, int&&) memory:3033
    #2 0x1023b66be in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:435
    #3 0x10239f116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x10239d17e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x1023ae0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x10239ea03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x1023e18cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x102446663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x102214e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x102051108 in main SkSLMain.cpp:258
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x1dad9) in wrap_memmove+0x169
Shadow bytes around the buggy address:
  0x1c2200005600: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c2200005610: 00 00 00 00 00 fa fa fa fa fa fa fa fa fa fa fa
  0x1c2200005620: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c2200005630: 00 00 00 00 00 00 00 00 00 00 00 00 00 fa fa fa
  0x1c2200005640: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
=>0x1c2200005650: fd fd fd fd fd fd[fd]fd fd fd fd fd fd fd fd fd
  0x1c2200005660: fd fd fd fd fd fd fa fa fa fa fa fa fa fa fa fa
  0x1c2200005670: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x1c2200005680: fd fd fd fd fd fd fd fd fd fd fd fd fd fa fa fa
  0x1c2200005690: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
  0x1c22000056a0: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
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
==77602==ABORTING
_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX half(x) 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX half4(half(x)) 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half(x))) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float _2_z = <undefined>, float[2] _1_y = <undefined>, float _3_0_foo = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x61b000003180): 10.0
Node 1 (0x61b000003198): float x = 10.0
Node 2 (0x61b0000031b0): float x = 10.0;
Node 3 (0x61b0000031c8): float[2] _1_y[2]
Node 4 (0x61b0000031e0): float _2_z
Node 5 (0x61b0000031f8): float _1_y, _2_z;
Node 6 (0x61b000003210): _1_y
Node 7 (0x61b000003228): 0
Node 8 (0x61b000003240): _1_y[0]
Node 9 (0x61b000003258): 10.0
Node 10 (0x61b000003270): (_1_y[0] = 10.0)
Node 11 (0x61b000003288): (_1_y[0] = 10.0);
Node 12 (0x61b0000032a0): _1_y
Node 13 (0x61b0000032b8): 1
Node 14 (0x61b0000032d0): _1_y[1]
Node 15 (0x61b0000032e8): 20.0
Node 16 (0x61b000003300): (_1_y[1] = 20.0)
Node 17 (0x61b000003318): (_1_y[1] = 20.0);
Node 18 (0x61b000003330): float _3_0_foo
Node 19 (0x61b000003348): float _3_0_foo;
Node 20 (0x61b000003360): _3_0_foo
Node 21 (0x61b000003378): _1_y
Node 22 (0x61b000003390): 0
Node 23 (0x61b0000033a8): _1_y[0]
Node 24 (0x61b0000033c0): _1_y
Node 25 (0x61b0000033d8): 1
Node 26 (0x61b0000033f0): _1_y[1]
Node 27 (0x61b000003408): (_1_y[0] * _1_y[1])
Node 28 (0x61b000003420): (_3_0_foo = (_1_y[0] * _1_y[1]))
Node 29 (0x61b000003438): (_3_0_foo = (_1_y[0] * _1_y[1]));
Node 30 (0x61b000003450): _2_z
Node 31 (0x61b000003468): _3_0_foo
Node 32 (0x61b000003480): (_2_z = _3_0_foo)
Node 33 (0x61b000003498): (_2_z = _3_0_foo);
Node 34 (0x61b0000034b0): x
Node 35 (0x61b0000034c8): _2_z
Node 36 (0x61b0000034e0): (x = _2_z)
Node 37 (0x61b0000034f8): (x = _2_z);
Node 38 (0x61b000003510): ;
Node 39 (0x61b000003528): sk_FragColor
Node 40 (0x61b000003540): x
Node 41 (0x61b000003558): half(x)
Node 42 (0x61b000003570): half4(half(x))
Node 43 (0x61b000003588): (sk_FragColor = half4(half(x)))
Node 44 (0x61b0000035a0): (sk_FragColor = half4(half(x)));
Exits: [1]

Block 1
-------
Before: [float _2_z = _3_0_foo, float[2] _1_y = <defined>, float _3_0_foo = (_1_y[0] * _1_y[1]), float x = _2_z]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half(x))); 
simplifying expr
Block 0
-------
Before: [float _2_z = <undefined>
