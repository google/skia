### Compilation failed:

Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000106a0): 0.0625
Node 1 (0x6080000106b8): half4(0.0625)
Node 2 (0x6080000106d0): return half4(0.0625);
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

about to simplify EX 0.0625 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000106a0): 0.0625
Node 1 (0x6080000106b8): half4(0.0625)
Node 2 (0x6080000106d0): return half4(0.0625);
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

about to simplify EX half4(0.0625) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000106a0): 0.0625
Node 1 (0x6080000106b8): half4(0.0625)
Node 2 (0x6080000106d0): return half4(0.0625);
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

about to simplify ST return half4(0.0625); 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): v.x
Node 2 (0x610000000d70): 0.5
Node 3 (0x610000000d88): (v.x < 0.5)
Node 4 (0x610000000da0): return (v.x < 0.5);
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
Node 1 (0x610000000d58): v.x
Node 2 (0x610000000d70): 0.5
Node 3 (0x610000000d88): (v.x < 0.5)
Node 4 (0x610000000da0): return (v.x < 0.5);
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

about to simplify EX v.x 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): v.x
Node 2 (0x610000000d70): 0.5
Node 3 (0x610000000d88): (v.x < 0.5)
Node 4 (0x610000000da0): return (v.x < 0.5);
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

about to simplify EX 0.5 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): v.x
Node 2 (0x610000000d70): 0.5
Node 3 (0x610000000d88): (v.x < 0.5)
Node 4 (0x610000000da0): return (v.x < 0.5);
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

about to simplify EX (v.x < 0.5) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000d40): v
Node 1 (0x610000000d58): v.x
Node 2 (0x610000000d70): 0.5
Node 3 (0x610000000d88): (v.x < 0.5)
Node 4 (0x610000000da0): return (v.x < 0.5);
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

about to simplify ST return (v.x < 0.5); 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000e40): v
Node 1 (0x610000000e58): 0.125
Node 2 (0x610000000e70): half4(0.125)
Node 3 (0x610000000e88): (v + half4(0.125))
Node 4 (0x610000000ea0): return (v + half4(0.125));
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
Node 0 (0x610000000e40): v
Node 1 (0x610000000e58): 0.125
Node 2 (0x610000000e70): half4(0.125)
Node 3 (0x610000000e88): (v + half4(0.125))
Node 4 (0x610000000ea0): return (v + half4(0.125));
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

about to simplify EX 0.125 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000e40): v
Node 1 (0x610000000e58): 0.125
Node 2 (0x610000000e70): half4(0.125)
Node 3 (0x610000000e88): (v + half4(0.125))
Node 4 (0x610000000ea0): return (v + half4(0.125));
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

about to simplify EX half4(0.125) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000e40): v
Node 1 (0x610000000e58): 0.125
Node 2 (0x610000000e70): half4(0.125)
Node 3 (0x610000000e88): (v + half4(0.125))
Node 4 (0x610000000ea0): return (v + half4(0.125));
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

about to simplify EX (v + half4(0.125)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x610000000e40): v
Node 1 (0x610000000e58): 0.125
Node 2 (0x610000000e70): half4(0.125)
Node 3 (0x610000000e88): (v + half4(0.125))
Node 4 (0x610000000ea0): return (v + half4(0.125));
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

about to simplify ST return (v + half4(0.125)); 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): _0_initLoopVar
Node 9 (0x613000001d18): (sk_FragColor = _0_initLoopVar)
Node 10 (0x613000001d30): (sk_FragColor = _0_initLoopVar);
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify ST half4 _0_initLoopVar 
simplifying vardecl
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): _0_initLoopVar
Node 9 (0x613000001d18): (sk_FragColor = _0_initLoopVar)
Node 10 (0x613000001d30): (sk_FragColor = _0_initLoopVar);
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify ST half4 _0_initLoopVar; 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): _0_initLoopVar
Node 9 (0x613000001d18): (sk_FragColor = _0_initLoopVar)
Node 10 (0x613000001d30): (sk_FragColor = _0_initLoopVar);
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX _0_initLoopVar 
optimizing varref 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): _0_initLoopVar
Node 9 (0x613000001d18): (sk_FragColor = _0_initLoopVar)
Node 10 (0x613000001d30): (sk_FragColor = _0_initLoopVar);
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX 0.0625 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): _0_initLoopVar
Node 9 (0x613000001d18): (sk_FragColor = _0_initLoopVar)
Node 10 (0x613000001d30): (sk_FragColor = _0_initLoopVar);
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX half4(0.0625) 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): _0_initLoopVar
Node 9 (0x613000001d18): (sk_FragColor = _0_initLoopVar)
Node 10 (0x613000001d30): (sk_FragColor = _0_initLoopVar);
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX (_0_initLoopVar = half4(0.0625)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): _0_initLoopVar
Node 9 (0x613000001d18): (sk_FragColor = _0_initLoopVar)
Node 10 (0x613000001d30): (sk_FragColor = _0_initLoopVar);
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify ST (_0_initLoopVar = half4(0.0625)); 
simplifying expr
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): _0_initLoopVar
Node 9 (0x613000001d18): (sk_FragColor = _0_initLoopVar)
Node 10 (0x613000001d30): (sk_FragColor = _0_initLoopVar);
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): _0_initLoopVar
Node 9 (0x613000001d18): (sk_FragColor = _0_initLoopVar)
Node 10 (0x613000001d30): (sk_FragColor = _0_initLoopVar);
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX _0_initLoopVar 
optimized to half4(0.0625) 
coerced to half4(0.0625) 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX (sk_FragColor = half4(0.0625)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify ST (sk_FragColor = half4(0.0625)); 
simplifying expr
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX shouldLoop(sk_FragColor) 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX grow(sk_FragColor) 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX (sk_FragColor = grow(sk_FragColor)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): half4 _0_initLoopVar
Node 1 (0x613000001c58): half4 _0_initLoopVar;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify ST half4 _0_initLoopVar 
simplifying vardecl
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): ;
Node 1 (0x613000001c58): half4 ;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify ST half4 ; 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): ;
Node 1 (0x613000001c58): half4 ;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX _0_initLoopVar 
optimizing varre=================================================================
==77145==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000019ef0 at pc 0x000106599c05 bp 0x7ffee977be10 sp 0x7ffee977be08
READ of size 8 at 0x602000019ef0 thread T0
    #0 0x106599c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10659abad in SkSL::BasicBlock::Node::description() const SkSLCFGGenerator.h:65
    #2 0x106599069 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:79
    #3 0x106597fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #4 0x10663dd91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #5 0x10664c657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #6 0x1066496e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #7 0x106484886 in main SkSLMain.cpp:242
    #8 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x602000019ef0 is located 0 bytes inside of 16-byte region [0x602000019ef0,0x602000019f00)
freed by thread T0 here:
    #0 0x1081b6c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x1064a1f9c in std::__1::_DeallocateCaller::__do_call(void*) new:334
    #2 0x1064a1f40 in std::__1::_DeallocateCaller::__do_deallocate_handle_size(void*, unsigned long) new:292
    #3 0x10689645c in std::__1::_DeallocateCaller::__do_deallocate_handle_size_align(void*, unsigned long, unsigned long) new:268
    #4 0x1068963ec in std::__1::__libcpp_deallocate(void*, unsigned long, unsigned long) new:340
    #5 0x1068ef8f1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::deallocate(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1872
    #6 0x1068ef4dc in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::deallocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1594
    #7 0x1068eeeab in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:464
    #8 0x106c4f29d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #9 0x106c49371 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #10 0x106c492a0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #11 0x1066794b4 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #12 0x10689bda4 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #13 0x106899eb1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #14 0x106899f11 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #15 0x1066e41f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #16 0x1066e3bdd in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #17 0x10663015d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #18 0x106631e24 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719
    #19 0x10662e4b7 in SkSL::Compiler::simplifyExpression(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:925
    #20 0x10663deff in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1519
    #21 0x10664c657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #22 0x1066496e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #23 0x106484886 in main SkSLMain.cpp:242
    #24 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x1081b684d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x106894e64 in std::__1::__libcpp_allocate(unsigned long, unsigned long) new:253
    #2 0x1068f34b1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::allocate(unsigned long, void const*) memory:1869
    #3 0x1068f2fa8 in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::allocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, unsigned long) memory:1586
    #4 0x1068f2c0c in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:318
    #5 0x1068f1d2b in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:317
    #6 0x106873341 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::reserve(unsigned long) vector:1586
    #7 0x106898af4 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:55
    #8 0x1068054e7 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:54
    #9 0x1069cab5b in std::__1::__unique_if<SkSL::BinaryExpression>::__unique_single std::__1::make_unique<SkSL::BinaryExpression, int&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*>(int&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Token::Kind&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Type const*&&) memory:3033
    #10 0x1069d1f24 in SkSL::Inliner::inlineStatement(int, std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > > >*, SkSL::SymbolTable*, SkSL::Expression const*, bool, SkSL::Statement const&) SkSLInliner.cpp:487
    #11 0x1069e2b84 in SkSL::Inliner::inlineCall(SkSL::FunctionCall*, SkSL::SymbolTable*) SkSLInliner.cpp:686
    #12 0x10680892f in SkSL::IRGenerator::call(int, SkSL::FunctionDeclaration const&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2115
    #13 0x10680c503 in SkSL::IRGenerator::call(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2206
    #14 0x10682b521 in SkSL::IRGenerator::convertCallExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:2742
    #15 0x1067f6eaa in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1318
    #16 0x106828742 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1920
    #17 0x1067f6c39 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #18 0x1067dde58 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #19 0x1067d157b in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #20 0x1067e20e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #21 0x1067d525a in SkSL::IRGenerator::convertFor(SkSL::ASTNode const&) SkSLIRGenerator.cpp:558
    #22 0x1067d1248 in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:226
    #23 0x1067e20e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #24 0x1067d2a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #25 0x1068158cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #26 0x10687a663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #27 0x106648e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #28 0x106484886 in main SkSLMain.cpp:242
    #29 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c0400003380: fa fa 04 fa fa fa fd fa fa fa 00 00 fa fa 00 fa
  0x1c0400003390: fa fa fd fd fa fa fd fa fa fa 00 fa fa fa 00 00
  0x1c04000033a0: fa fa 00 fa fa fa fd fa fa fa 00 00 fa fa 00 fa
  0x1c04000033b0: fa fa fd fa fa fa fd fa fa fa fd fa fa fa fd fa
  0x1c04000033c0: fa fa 00 fa fa fa 00 00 fa fa 00 fa fa fa fd fa
=>0x1c04000033d0: fa fa 00 fa fa fa 00 fa fa fa 00 fa fa fa[fd]fd
  0x1c04000033e0: fa fa fd fa fa fa 00 00 fa fa 00 00 fa fa fd fa
  0x1c04000033f0: fa fa fd fa fa fa fd fa fa fa fd fa fa fa 00 fa
  0x1c0400003400: fa fa fd fa fa fa fd fa fa fa fd fa fa fa fd fa
  0x1c0400003410: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 00
  0x1c0400003420: fa fa 00 fa fa fa fd fa fa fa fd fa fa fa 00 00
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
==77145==ABORTING
f 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): ;
Node 1 (0x613000001c58): half4 ;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX 0.0625 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): ;
Node 1 (0x613000001c58): half4 ;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX half4(0.0625) 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): ;
Node 1 (0x613000001c58): half4 ;
Node 2 (0x613000001c70): _0_initLoopVar
Node 3 (0x613000001c88): 0.0625
Node 4 (0x613000001ca0): half4(0.0625)
Node 5 (0x613000001cb8): (_0_initLoopVar = half4(0.0625))
Node 6 (0x613000001cd0): (_0_initLoopVar = half4(0.0625));
Node 7 (0x613000001ce8): sk_FragColor
Node 8 (0x613000001d00): 0.0625
Node 9 (0x613000001d18): half4(0.0625)
Node 10 (0x613000001d30): (sk_FragColor = half4(0.0625))
Node 11 (0x613000001d48): (sk_FragColor = half4(0.0625));
Exits: [1]

Block 1
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [0, 2]
Node 0 (0x60400000bed0): sk_FragColor
Node 1 (0x60400000bee8): shouldLoop(sk_FragColor)
Exits: [4]

Block 2
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [4]
Node 0 (0x6080000108a0): sk_FragColor
Node 1 (0x6080000108b8): sk_FragColor
Node 2 (0x6080000108d0): grow(sk_FragColor)
Node 3 (0x6080000108e8): (sk_FragColor = grow(sk_FragColor))
Exits: [1, 3]

Block 3
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [2]
Exits: [5]

Block 4
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [1]
Exits: [2]

Block 5
-------
Before: [half4 _0_initLoopVar = half4(0.0625)]
Entrances: [3]
Exits: []

about to simplify EX (_0_initLoopVar = half4(0.0625)) 
optimizing binary 
deadass? update=1 rescan=0 
deadass! update=1 rescan=0 
delete_left update=1 rescan=0 
Block 0
-------
Before: [half4 _0_initLoopVar = <undefined>]
Entrances: []
Node 0 (0x613000001c40): ;
Node 1 (0x613000001c58): half4 ;
