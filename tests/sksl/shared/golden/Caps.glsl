### Compilation failed:

Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify ST int x = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify ST int x = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify ST int y = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify ST int y = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify ST int z = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify ST int z = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): sk_Caps.externalTextureSupport
Node 10 (0x613000001b70): if (sk_Caps.externalTextureSupport) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify EX sk_Caps.externalTextureSupport 
optimized to true 
coerced to true 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0
Node 1 (0x613000001a98): int x = 0
Node 2 (0x613000001ab0): int x = 0;
Node 3 (0x613000001ac8): 0
Node 4 (0x613000001ae0): int y = 0
Node 5 (0x613000001af8): int y = 0;
Node 6 (0x613000001b10): 0
Node 7 (0x613000001b28): int z = 0
Node 8 (0x613000001b40): int z = 0;
Node 9 (0x613000001b58): true
Node 10 (0x613000001b70): if (true) (x = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 0]
Entrances: [0]
Node 0 (0x6080000106a0): x
Node 1 (0x6080000106b8): 1
Node 2 (0x6080000106d0): (x = 1)
Node 3 (0x6080000106e8): (x = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [0, 1]
Node 0 (0x60400000bad0): sk_Caps.fbFetchSupport
Node 1 (0x60400000bae8): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = 0, int x = <defined>]
Entrances: [2]
Node 0 (0x608000010720): y
Node 1 (0x608000010738): 1
Node 2 (0x608000010750): (y = 1)
Node 3 (0x608000010768): (y = 1);
Exits: [4]

Block 4
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [2, 3]
Node 0 (0x60400000bcd0): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000bce8): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [5, 6]

Block 5
-------
Before: [int z = 0, int y = <defined>, int x = <defined>]
Entrances: [4]
Node 0 (0x6080000107a0): z
Node 1 (0x6080000107b8): 1
Node 2 (0x6080000107d0): (z = 1)
Node 3 (0x6080000107e8): (z = 1);
Exits: [6]

Block 6
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [4, 5]
Node 0 (0x613000001c40): sk_FragColor
Node 1 (0x613000001c58): sk_FragColor.xyz
Node 2 (0x613000001c70): x
Node 3 (0x613000001c88): half(x)
Node 4 (0x613000001ca0): y
Node 5 (0x613000001cb8): half(y)
Node 6 (0x613000001cd0): z
Node 7 (0x613000001ce8): half(z)
Node 8 (0x613000001d00): half3(half(x), half(y), half(z))
Node 9 (0x613000001d18): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000001d30): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [7]

Block 7
-------
Before: [int z = <defined>, int y = <defined>, int x = <defined>]
Entrances: [6]
Exits: []

about to simplify ST if (true) (x = 1); 
simplifying if
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify ST int x = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify ST int x = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify ST int y = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify ST int y = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify ST int z = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify ST int z = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify EX (x = 1) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify ST (x = 1); 
simplifying expr
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): sk_Caps.fbFetchSupport
Node 14 (0x613000001f50): if (sk_Caps.fbFetchSupport) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify EX sk_Caps.fbFetchSupport 
optimized to true 
coerced to true 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x613000001e00): 0
Node 1 (0x613000001e18): int x = 0
Node 2 (0x613000001e30): int x = 0;
Node 3 (0x613000001e48): 0
Node 4 (0x613000001e60): int y = 0
Node 5 (0x613000001e78): int y = 0;
Node 6 (0x613000001e90): 0
Node 7 (0x613000001ea8): int z = 0
Node 8 (0x613000001ec0): int z = 0;
Node 9 (0x613000001ed8): x
Node 10 (0x613000001ef0): 1
Node 11 (0x613000001f08): (x = 1)
Node 12 (0x613000001f20): (x = 1);
Node 13 (0x613000001f38): true
Node 14 (0x613000001f50): if (true) (y = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 0, int x = 1]
Entrances: [0]
Node 0 (0x6080000109a0): y
Node 1 (0x6080000109b8): 1
Node 2 (0x6080000109d0): (y = 1)
Node 3 (0x6080000109e8): (y = 1);
Exits: [2]

Block 2
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [0, 1]
Node 0 (0x60400000f010): sk_Caps.canUseAnyFunctionInShader
Node 1 (0x60400000f028): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [3, 4]

Block 3
-------
Before: [int z = 0, int y = <defined>, int x = 1]
Entrances: [2]
Node 0 (0x608000010a20): z
Node 1 (0x608000010a38): 1
Node 2 (0x608000010a50): (z = 1)
Node 3 (0x608000010a68): (z = 1);
Exits: [4]

Block 4
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [2, 3]
Node 0 (0x613000001fc0): sk_FragColor
Node 1 (0x613000001fd8): sk_FragColor.xyz
Node 2 (0x613000001ff0): x
Node 3 (0x613000002008): half(x)
Node 4 (0x613000002020): y
Node 5 (0x613000002038): half(y)
Node 6 (0x613000002050): z
Node 7 (0x613000002068): half(z)
Node 8 (0x613000002080): half3(half(x), half(y), half(z))
Node 9 (0x613000002098): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x6130000020b0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [5]

Block 5
-------
Before: [int z = <defined>, int y = <defined>, int x = 1]
Entrances: [4]
Exits: []

about to simplify ST if (true) (y = 1); 
simplifying if
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify ST int x = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify ST int x = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify ST int y = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify ST int y = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify ST int z = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify ST int z = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX (x = 1) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify ST (x = 1); 
simplifying expr
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX y 
optimizing varref 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX (y = 1) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify ST (y = 1); 
simplifying expr
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): sk_Caps.canUseAnyFunctionInShader
Node 18 (0x6170000021b0): if (sk_Caps.canUseAnyFunctionInShader) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify EX sk_Caps.canUseAnyFunctionInShader 
optimized to false 
coerced to false 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002000): 0
Node 1 (0x617000002018): int x = 0
Node 2 (0x617000002030): int x = 0;
Node 3 (0x617000002048): 0
Node 4 (0x617000002060): int y = 0
Node 5 (0x617000002078): int y = 0;
Node 6 (0x617000002090): 0
Node 7 (0x6170000020a8): int z = 0
Node 8 (0x6170000020c0): int z = 0;
Node 9 (0x6170000020d8): x
Node 10 (0x6170000020f0): 1
Node 11 (0x617000002108): (x = 1)
Node 12 (0x617000002120): (x = 1);
Node 13 (0x617000002138): y
Node 14 (0x617000002150): 1
Node 15 (0x617000002168): (y = 1)
Node 16 (0x617000002180): (y = 1);
Node 17 (0x617000002198): false
Node 18 (0x6170000021b0): if (false) (z = 1);
Exits: [1, 2]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Node 0 (0x608000010c20): z
Node 1 (0x608000010c38): 1
Node 2 (0x608000010c50): (z = 1)
Node 3 (0x608000010c68): (z = 1);
Exits: [2]

Block 2
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [0, 1]
Node 0 (0x613000002340): sk_FragColor
Node 1 (0x613000002358): sk_FragColor.xyz
Node 2 (0x613000002370): x
Node 3 (0x613000002388): half(x)
Node 4 (0x6130000023a0): y
Node 5 (0x6130000023b8): half(y)
Node 6 (0x6130000023d0): z
Node 7 (0x6130000023e8): half(z)
Node 8 (0x613000002400): half3(half(x), half(y), half(z))
Node 9 (0x613000002418): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 10 (0x613000002430): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [3]

Block 3
-------
Before: [int z = <defined>, int y = 1, int x = 1]
Entrances: [2]
Exits: []

about to simplify ST if (false) (z = 1); 
simplifying if
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int x = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int x = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int y = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int y = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int z = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int z = 0; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX (x = 1) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST (x = 1); 
simplifying expr
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX y 
optimizing varref 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX (y = 1) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST (y = 1); 
simplifying expr
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor.xyz 
optimizing swiz 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): x
Node 20 (0x617000002560): half(x)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(x), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(x), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(x), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to 1 
coerced to 1 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1
Node 20 (0x617000002560): half(1)
Node 21 (0x617000002578): y
Node 22 (0x617000002590): half(y)
Node 23 (0x6170000025a8): z
Node 24 (0x6170000025c0): half(z)
Node 25 (0x6170000025d8): half3(half(1), half(y), half(z))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(half(1), half(y), half(z)))
Node 27 (0x617000002608): (sk_FragColor.xyz = half3(half(1), half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX half(1) 
optimized to 1.0 
coerced to 1.0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): y
Node 21 (0x617000002578): half(y)
Node 22 (0x617000002590): z
Node 23 (0x6170000025a8): half(z)
Node 24 (0x6170000025c0): half3(1.0, half(y), half(z))
Node 25 (0x6170000025d8): (sk_FragColor.xyz = half3(1.0, half(y), half(z)))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(1.0, half(y), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX y 
optimized to 1 
coerced to 1 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 1
Node 21 (0x617000002578): half(1)
Node 22 (0x617000002590): z
Node 23 (0x6170000025a8): half(z)
Node 24 (0x6170000025c0): half3(1.0, half(1), half(z))
Node 25 (0x6170000025d8): (sk_FragColor.xyz = half3(1.0, half(1), half(z)))
Node 26 (0x6170000025f0): (sk_FragColor.xyz = half3(1.0, half(1), half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX half(1) 
optimized to 1.0 
coerced to 1.0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 1.0
Node 21 (0x617000002578): z
Node 22 (0x617000002590): half(z)
Node 23 (0x6170000025a8): half3(1.0, 1.0, half(z))
Node 24 (0x6170000025c0): (sk_FragColor.xyz = half3(1.0, 1.0, half(z)))
Node 25 (0x6170000025d8): (sk_FragColor.xyz = half3(1.0, 1.0, half(z)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX z 
optimized to 0 
coerced to 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 1.0
Node 21 (0x617000002578): 0
Node 22 (0x617000002590): half(0)
Node 23 (0x6170000025a8): half3(1.0, 1.0, half(0))
Node 24 (0x6170000025c0): (sk_FragColor.xyz = half3(1.0, 1.0, half(0)))
Node 25 (0x6170000025d8): (sk_FragColor.xyz = half3(1.0, 1.0, half(0)));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX half(0) 
optimized to 0.0 
coerced to 0.0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 1.0
Node 21 (0x617000002578): 0.0
Node 22 (0x617000002590): half3(1.0, 1.0, 0.0)
Node 23 (0x6170000025a8): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 24 (0x6170000025c0): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX half3(1.0, 1.0, 0.0) 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 1.0
Node 21 (0x617000002578): 0.0
Node 22 (0x617000002590): half3(1.0, 1.0, 0.0)
Node 23 (0x6170000025a8): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 24 (0x6170000025c0): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor.xyz = half3(1.0, 1.0, 0.0)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 1.0
Node 21 (0x617000002578): 0.0
Node 22 (0x617000002590): half3(1.0, 1.0, 0.0)
Node 23 (0x6170000025a8): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 24 (0x6170000025c0): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor.xyz = half3(1.0, 1.0, 0.0)); 
simplifying expr
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 1.0
Node 21 (0x617000002578): 0.0
Node 22 (0x617000002590): half3(1.0, 1.0, 0.0)
Node 23 (0x6170000025a8): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 24 (0x6170000025c0): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): 0
Node 1 (0x617000002398): int x = 0
Node 2 (0x6170000023b0): int x = 0;
Node 3 (0x6170000023c8): 0
Node 4 (0x6170000023e0): int y = 0
Node 5 (0x6170000023f8): int y = 0;
Node 6 (0x617000002410): 0
Node 7 (0x617000002428): int z = 0
Node 8 (0x617000002440): int z = 0;
Node 9 (0x617000002458): x
Node 10 (0x617000002470): 1
Node 11 (0x617000002488): (x = 1)
Node 12 (0x6170000024a0): (x = 1);
Node 13 (0x6170000024b8): y
Node 14 (0x6170000024d0): 1
Node 15 (0x6170000024e8): (y = 1)
Node 16 (0x617000002500): (y = 1);
Node 17 (0x617000002518): sk_FragColor
Node 18 (0x617000002530): sk_FragColor.xyz
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 1.0
Node 21 (0x617000002578): 0.0
Node 22 (0x617000002590): half3(1.0, 1.0, 0.0)
Node 23 (0x6170000025a8): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 24 (0x6170000025c0): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int x = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): ;
Node 1 (0x617000002398): int ;
Node 2 (0x6170000023b0): 0
Node 3 (0x6170000023c8): int y = 0
Node 4 (0x6170000023e0): int y = 0;
Node 5 (0x6170000023f8): 0
Node 6 (0x617000002410): int z = 0
Node 7 (0x617000002428): int z = 0;
Node 8 (0x617000002440): x
Node 9 (0x617000002458): 1
Node 10 (0x617000002470): (x = 1)
Node 11 (0x617000002488): (x = 1);
Node 12 (0x6170000024a0): y
Node 13 (0x6170000024b8): 1
Node 14 (0x6170000024d0): (y = 1)
Node 15 (0x6170000024e8): (y = 1);
Node 16 (0x617000002500): sk_FragColor
Node 17 (0x617000002518): sk_FragColor.xyz
Node 18 (0x617000002530): 1.0
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 0.0
Node 21 (0x617000002578): half3(1.0, 1.0, 0.0)
Node 22 (0x617000002590): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 23 (0x6170000025a8): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int ; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): ;
Node 1 (0x617000002398): int ;
Node 2 (0x6170000023b0): 0
Node 3 (0x6170000023c8): int y = 0
Node 4 (0x6170000023e0): int y = 0;
Node 5 (0x6170000023f8): 0
Node 6 (0x617000002410): int z = 0
Node 7 (0x617000002428): int z = 0;
Node 8 (0x617000002440): x
Node 9 (0x617000002458): 1
Node 10 (0x617000002470): (x = 1)
Node 11 (0x617000002488): (x = 1);
Node 12 (0x6170000024a0): y
Node 13 (0x6170000024b8): 1
Node 14 (0x6170000024d0): (y = 1)
Node 15 (0x6170000024e8): (y = 1);
Node 16 (0x617000002500): sk_FragColor
Node 17 (0x617000002518): sk_FragColor.xyz
Node 18 (0x617000002530): 1.0
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 0.0
Node 21 (0x617000002578): half3(1.0, 1.0, 0.0)
Node 22 (0x617000002590): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 23 (0x6170000025a8): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): ;
Node 1 (0x617000002398): int ;
Node 2 (0x6170000023b0): 0
Node 3 (0x6170000023c8): int y = 0
Node 4 (0x6170000023e0): int y = 0;
Node 5 (0x6170000023f8): 0
Node 6 (0x617000002410): int z = 0
Node 7 (0x617000002428): int z = 0;
Node 8 (0x617000002440): x
Node 9 (0x617000002458): 1
Node 10 (0x617000002470): (x = 1)
Node 11 (0x617000002488): (x = 1);
Node 12 (0x6170000024a0): y
Node 13 (0x6170000024b8): 1
Node 14 (0x6170000024d0): (y = 1)
Node 15 (0x6170000024e8): (y = 1);
Node 16 (0x617000002500): sk_FragColor
Node 17 (0x617000002518): sk_FragColor.xyz
Node 18 (0x617000002530): 1.0
Node 19 (0x617000002548): 1.0
Node 20 (0x617000002560): 0.0
Node 21 (0x617000002578): half3(1.0, 1.0, 0.0)
Node 22 (0x617000002590): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 23 (0x6170000025a8): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int y = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): ;
Node 1 (0x617000002398): int ;
Node 2 (0x6170000023b0): ;
Node 3 (0x6170000023c8): int ;
Node 4 (0x6170000023e0): 0
Node 5 (0x6170000023f8): int z = 0
Node 6 (0x617000002410): int z = 0;
Node 7 (0x617000002428): x
Node 8 (0x617000002440): 1
Node 9 (0x617000002458): (x = 1)
Node 10 (0x617000002470): (x = 1);
Node 11 (0x617000002488): y
Node 12 (0x6170000024a0): 1
Node 13 (0x6170000024b8): (y = 1)
Node 14 (0x6170000024d0): (y = 1);
Node 15 (0x6170000024e8): sk_FragColor
Node 16 (0x617000002500): sk_FragColor.xyz
Node 17 (0x617000002518): 1.0
Node 18 (0x617000002530): 1.0
Node 19 (0x617000002548): 0.0
Node 20 (0x617000002560): half3(1.0, 1.0, 0.0)
Node 21 (0x617000002578): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 22 (0x617000002590): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int ; 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): ;
Node 1 (0x617000002398): int ;
Node 2 (0x6170000023b0): ;
Node 3 (0x6170000023c8): int ;
Node 4 (0x6170000023e0): 0
Node 5 (0x6170000023f8): int z = 0
Node 6 (0x617000002410): int z = 0;
Node 7 (0x617000002428): x
Node 8 (0x617000002440): 1
Node 9 (0x617000002458): (x = 1)
Node 10 (0x617000002470): (x = 1);
Node 11 (0x617000002488): y
Node 12 (0x6170000024a0): 1
Node 13 (0x6170000024b8): (y = 1)
Node 14 (0=================================================================
==77456==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013e70 at pc 0x000106ae1c05 bp 0x7ffee9233f20 sp 0x7ffee9233f18
READ of size 8 at 0x60d000013e70 thread T0
    #0 0x106ae1c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x106ae09a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x106adffcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x106b85d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x106b94657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x106b916e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x1069cc886 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013e70 is located 128 bytes inside of 136-byte region [0x60d000013df0,0x60d000013e78)
freed by thread T0 here:
    #0 0x1086f9c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x106e698dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x106ca6e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x106b8108d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x106bcc97d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x106b80a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x106b7c4a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x106b86012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x106b94657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x106b916e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x1069cc886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x1086f984d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x106d41fdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x106d349a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x106d1b116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x106d1917e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x106d2a0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x106d1aa03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x106d5d8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x106dc2663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x106b90e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x1069cc886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a00002770: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002780: 00 fa fa fa fa fa fa fa fa fa fd fd fd fd fd fd
  0x1c1a00002790: fd fd fd fd fd fd fd fd fd fd fd fa fa fa fa fa
  0x1c1a000027a0: fa fa fa fa fd fd fd fd fd fd fd fd fd fd fd fd
  0x1c1a000027b0: fd fd fd fd fd fa fa fa fa fa fa fa fa fa fd fd
=>0x1c1a000027c0: fd fd fd fd fd fd fd fd fd fd fd fd fd fd[fd]fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027e0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027f0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a00002800: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a00002810: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==77456==ABORTING
x6170000024d0): (y = 1);
Node 15 (0x6170000024e8): sk_FragColor
Node 16 (0x617000002500): sk_FragColor.xyz
Node 17 (0x617000002518): 1.0
Node 18 (0x617000002530): 1.0
Node 19 (0x617000002548): 0.0
Node 20 (0x617000002560): half3(1.0, 1.0, 0.0)
Node 21 (0x617000002578): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 22 (0x617000002590): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): ;
Node 1 (0x617000002398): int ;
Node 2 (0x6170000023b0): ;
Node 3 (0x6170000023c8): int ;
Node 4 (0x6170000023e0): 0
Node 5 (0x6170000023f8): int z = 0
Node 6 (0x617000002410): int z = 0;
Node 7 (0x617000002428): x
Node 8 (0x617000002440): 1
Node 9 (0x617000002458): (x = 1)
Node 10 (0x617000002470): (x = 1);
Node 11 (0x617000002488): y
Node 12 (0x6170000024a0): 1
Node 13 (0x6170000024b8): (y = 1)
Node 14 (0x6170000024d0): (y = 1);
Node 15 (0x6170000024e8): sk_FragColor
Node 16 (0x617000002500): sk_FragColor.xyz
Node 17 (0x617000002518): 1.0
Node 18 (0x617000002530): 1.0
Node 19 (0x617000002548): 0.0
Node 20 (0x617000002560): half3(1.0, 1.0, 0.0)
Node 21 (0x617000002578): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 22 (0x617000002590): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [int z = 0, int y = 1, int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int z = 0 
simplifying vardecl
Block 0
-------
Before: [int z = <undefined>, int y = <undefined>, int x = <undefined>]
Entrances: []
Node 0 (0x617000002380): ;
Node 1 (0x617000002398): int ;
Node 2 (0x6170000023b0): ;
Node 3 (0x6170000023c8): int ;
Node 4 (0x6170000023e0): ;
Node 5 (0x6170000023f8): int ;
Node 6 (0x617000002410): x
Node 7 (0x617000002428): 1
Node 8 (0x617000002440): (x = 1)
Node 9 (0x617000002458): (x = 1);
Node 10 (0x617000002470): y
Node 11 (0x617000002488): 1
Node 12 (0x6170000024a0): (y = 1)
Node 13 (0x6170000024b8): (y = 1);
Node 14 (0x6170000024d0): sk_FragColor
Node 15 (0x6170000024e8): sk_FragColor.xyz
Node 16 (0x617000002500): 1.0
Node 17 (0x617000002518): 1.0
Node 18 (0x617000002530): 0.0
Node 19 (0x617000002548): half3(1.0, 1.0, 0.0)
Node 20 (0x617000002560): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0))
Node 21 (0x617000002578): (sk_FragColor.xyz = half3(1.0, 1.0, 0.0));
Exits: [1]

Block 1
-------
Before: [
