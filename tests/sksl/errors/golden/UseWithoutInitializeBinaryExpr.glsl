### Compilation failed:

Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): bool x
Node 1 (0x608000010138): bool x;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify ST bool x 
simplifying vardecl
Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): ;
Node 1 (0x608000010138): bool ;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify ST bool ; 
Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): ;
Node 1 (0x608000010138): bool ;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): ;
Node 1 (0x608000010138): bool ;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify ST if (x) return; 
simplifying if
Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): ;
Node 1 (0x608000010138): bool ;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify ST return; 
Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): ;
Node 1 (0x608000010138): bool ;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify ST ; 
Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): ;
Node 1 (0x608000010138): bool ;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify ST bool ; 
Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): ;
Node 1 (0x608000010138): bool ;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): ;
Node 1 (0x608000010138): bool ;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify ST if (x) return; 
simplifying if
Block 0
-------
Before: [bool x = <undefined>]
Entrances: []
Node 0 (0x608000010120): ;
Node 1 (0x608000010138): bool ;
Node 2 (0x608000010150): x
Node 3 (0x608000010168): if (x) return;
Exits: [1, 3]

Block 1
-------
Before: [bool x = <undefined>]
Entrances: [0]
Node 0 (0x60300000a300): return;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [bool x = <undefined>]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [bool x = <undefined>]
Entrances: [3]
Exits: []

about to simplify ST return; 
error: 1: 'x' has not been assigned
1 error
