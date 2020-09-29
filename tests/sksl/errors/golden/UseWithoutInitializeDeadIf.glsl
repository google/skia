### Compilation failed:

Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000c40): int x
Node 1 (0x610000000c58): int x;
Node 2 (0x610000000c70): x
Node 3 (0x610000000c88): x++
Node 4 (0x610000000ca0): x++;
Exits: [1]

Block 1
-------
Before: [int x = <defined>]
Entrances: [0]
Exits: []

about to simplify ST int x 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000c40): int x
Node 1 (0x610000000c58): int x;
Node 2 (0x610000000c70): x
Node 3 (0x610000000c88): x++
Node 4 (0x610000000ca0): x++;
Exits: [1]

Block 1
-------
Before: [int x = <defined>]
Entrances: [0]
Exits: []

about to simplify ST int x; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000c40): int x
Node 1 (0x610000000c58): int x;
Node 2 (0x610000000c70): x
Node 3 (0x610000000c88): x++
Node 4 (0x610000000ca0): x++;
Exits: [1]

Block 1
-------
Before: [int x = <defined>]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000c40): int x
Node 1 (0x610000000c58): int x;
Node 2 (0x610000000c70): x
Node 3 (0x610000000c88): x++
Node 4 (0x610000000ca0): x++;
Exits: [1]

Block 1
-------
Before: [int x = <defined>]
Entrances: [0]
Exits: []

about to simplify EX x++ 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000c40): int x
Node 1 (0x610000000c58): int x;
Node 2 (0x610000000c70): x
Node 3 (0x610000000c88): x++
Node 4 (0x610000000ca0): x++;
Exits: [1]

Block 1
-------
Before: [int x = <defined>]
Entrances: [0]
Exits: []

about to simplify ST x++; 
simplifying expr
error: 1: 'x' has not been assigned
1 error
