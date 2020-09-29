### Compilation failed:

Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): y
Node 9 (0x613000001998): (x < y)
Node 10 (0x6130000019b0): @if ((x < y)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX 25.0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): y
Node 9 (0x613000001998): (x < y)
Node 10 (0x6130000019b0): @if ((x < y)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX sqrt(25.0) 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): y
Node 9 (0x613000001998): (x < y)
Node 10 (0x6130000019b0): @if ((x < y)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify ST float x = sqrt(25.0) 
simplifying vardecl
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): y
Node 9 (0x613000001998): (x < y)
Node 10 (0x6130000019b0): @if ((x < y)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify ST float x = sqrt(25.0); 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): y
Node 9 (0x613000001998): (x < y)
Node 10 (0x6130000019b0): @if ((x < y)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX 10.0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): y
Node 9 (0x613000001998): (x < y)
Node 10 (0x6130000019b0): @if ((x < y)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify ST float y = 10.0 
simplifying vardecl
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): y
Node 9 (0x613000001998): (x < y)
Node 10 (0x6130000019b0): @if ((x < y)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify ST float y = 10.0; 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): y
Node 9 (0x613000001998): (x < y)
Node 10 (0x6130000019b0): @if ((x < y)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): y
Node 9 (0x613000001998): (x < y)
Node 10 (0x6130000019b0): @if ((x < y)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX y 
optimized to 10.0 
coerced to 10.0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX (x < 10.0) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify ST @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}
 
simplifying if
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX half4(1.0) 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX (sk_FragColor = half4(1.0)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x=================================================================
==76861==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013da0 at pc 0x0001020bac05 bp 0x7ffeedc5af00 sp 0x7ffeedc5aef8
READ of size 8 at 0x60d000013da0 thread T0
    #0 0x1020bac04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x1020b99a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x1020b8fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x10215ed91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x10216d657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x10216a6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x101fa5886 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013da0 is located 128 bytes inside of 136-byte region [0x60d000013d20,0x60d000013da8)
freed by thread T0 here:
    #0 0x103cd7c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x1024428dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x10227fe01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10215a08d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x1021a597d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x102159a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x1021554a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x10215f012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x10216d657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x10216a6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x101fa5886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x103cd784d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10231afdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x10230d9a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x1022f4116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x1022f217e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x1023030e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x1022f3a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x1023368cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x10239b663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x102169e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x101fa5886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a00002760: 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa fa
  0x1c1a00002770: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002780: 00 fa fa fa fa fa fa fa fa fa 00 00 00 00 00 00
  0x1c1a00002790: 00 00 00 00 00 00 00 00 00 00 00 fa fa fa fa fa
  0x1c1a000027a0: fa fa fa fa fd fd fd fd fd fd fd fd fd fd fd fd
=>0x1c1a000027b0: fd fd fd fd[fd]fa fa fa fa fa fa fa fa fa 00 00
  0x1c1a000027c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027e0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027f0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a00002800: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==76861==ABORTING
613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify ST (sk_FragColor = half4(1.0)); 
simplifying expr
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX 25.0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX sqrt(25.0) 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify ST float x = sqrt(25.0) 
simplifying vardecl
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify ST float x = sqrt(25.0); 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify EX 10.0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): 10.0
Node 5 (0x613000001938): float y = 10.0
Node 6 (0x613000001950): float y = 10.0;
Node 7 (0x613000001968): x
Node 8 (0x613000001980): 10.0
Node 9 (0x613000001998): (x < 10.0)
Node 10 (0x6130000019b0): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0]
Node 0 (0x610000000e40): sk_FragColor
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): half4(1.0)
Node 3 (0x610000000e88): (sk_FragColor = half4(1.0))
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0));
Exits: [2]

Block 2
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [0, 1]
Exits: [3]

Block 3
-------
Before: [float y = 10.0, float x = sqrt(25.0)]
Entrances: [2]
Exits: []

about to simplify ST float y = 10.0 
simplifying vardecl
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 25.0
Node 1 (0x6130000018d8): sqrt(25.0)
Node 2 (0x6130000018f0): float x = sqrt(25.0)
Node 3 (0x613000001908): float x = sqrt(25.0);
Node 4 (0x613000001920): ;
Node 5 (0x613000001938): float ;
Node 6 (0x613000001950): x
Node 7 (0x613000001968): 10.0
Node 8 (0x613000001980): (x < 10.0)
Node 9 (0x613000001998): @if ((x < 10.0)) {
(sk_FragColor = half4(1.0));
}

Exits: [1, 2]

Block 1
-------
Before: [
