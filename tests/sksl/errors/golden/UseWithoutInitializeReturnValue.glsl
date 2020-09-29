### Compilation failed:

Block 0
-------
Before: [int r = <undefined>]
Entrances: []
Node 0 (0x60800000ffa0): int r
Node 1 (0x60800000ffb8): int r;
Node 2 (0x60800000ffd0): r
Node 3 (0x60800000ffe8): return r;
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

about to simplify ST int r 
simplifying vardecl
Block 0
-------
Before: [int r = <undefined>]
Entrances: []
Node 0 (0x60800000ffa0): ;
Node 1 (0x60800000ffb8): int ;
Node 2 (0x60800000ffd0): r
Node 3 (0x60800000ffe8): return r;
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

about to simplify ST int ; 
Block 0
-------
Before: [int r = <undefined>]
Entrances: []
Node 0 (0x60800000ffa0): ;
Node 1 (0x60800000ffb8): int ;
Node 2 (0x60800000ffd0): r
Node 3 (0x60800000ffe8): return r;
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

about to simplify EX r 
optimizing varref 
Block 0
-------
Before: [int r = <undefined>]
Entrances: []
Node 0 (0x60800000ffa0): ;
Node 1 (0x60800000ffb8): int ;
Node 2 (0x60800000ffd0): r
Node 3 (0x60800000ffe8): return r;
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

about to simplify ST return r; 
Block 0
-------
Before: [int r = <undefined>]
Entrances: []
Node 0 (0x60800000ffa0): ;
Node 1 (0x60800000ffb8): int ;
Node 2 (0x60800000ffd0): r
Node 3 (0x60800000ffe8): return r;
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

about to simplify ST ; 
Block 0
-------
Before: [int r = <undefined>]
Entrances: []
Node 0 (0x60800000ffa0): ;
Node 1 (0x60800000ffb8): int ;
Node 2 (0x60800000ffd0): r
Node 3 (0x60800000ffe8): return r;
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

about to simplify ST int ; 
Block 0
-------
Before: [int r = <undefined>]
Entrances: []
Node 0 (0x60800000ffa0): ;
Node 1 (0x60800000ffb8): int ;
Node 2 (0x60800000ffd0): r
Node 3 (0x60800000ffe8): return r;
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

about to simplify EX r 
optimizing varref 
Block 0
-------
Before: [int r = <undefined>]
Entrances: []
Node 0 (0x60800000ffa0): ;
Node 1 (0x60800000ffb8): int ;
Node 2 (0x60800000ffd0): r
Node 3 (0x60800000ffe8): return r;
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

about to simplify ST return r; 
error: 1: 'r' has not been assigned
1 error
