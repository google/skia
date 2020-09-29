### Compilation failed:

Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX sqrt(1.0) 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX int(sqrt(1.0)) 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify ST int x = int(sqrt(1.0)) 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify ST int x = int(sqrt(1.0)); 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify ST @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
} 
simplifying switch
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX half4(1.0) 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX (sk_FragColor = half4(1.0)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify ST (sk_FragColor = half4(1.0)); 
simplifying expr
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify ST break; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX half4(0.0) 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify EX (sk_FragColor = half4(0.0)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1.0
Node 1 (0x610000000d58): sqrt(1.0)
Node 2 (0x610000000d70): int(sqrt(1.0))
Node 3 (0x610000000d88): int x = int(sqrt(1.0))
Node 4 (0x610000000da0): int x = int(sqrt(1.0));
Node 5 (0x610000000db8): x
Node 6 (0x610000000dd0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = int(sqrt(1.0))]
Entrances: [1]
Exits: []

about to simplify ST (sk_FragColor = half4(0.0)); 
simplifying expr
error: 3: static switch has non-static test
1 error
