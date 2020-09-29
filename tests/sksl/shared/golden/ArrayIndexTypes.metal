### Compilation failed:

Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short(0)
Node 9 (0x61b000002458): short x = short(0)
Node 10 (0x61b000002470): short x = short(0);
Node 11 (0x61b000002488): 1
Node 12 (0x61b0000024a0): ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1)
Node 14 (0x61b0000024d0): ushort y = ushort(1);
Node 15 (0x61b0000024e8): 2
Node 16 (0x61b000002500): int z = 2
Node 17 (0x61b000002518): int z = 2;
Node 18 (0x61b000002530): 3
Node 19 (0x61b000002548): uint w = 3
Node 20 (0x61b000002560): uint w = 3;
Node 21 (0x61b000002578): sk_FragColor
Node 22 (0x61b000002590): array
Node 23 (0x61b0000025a8): x
Node 24 (0x61b0000025c0): int(x)
Node 25 (0x61b0000025d8): array[int(x)]
Node 26 (0x61b0000025f0): half(array[int(x)])
Node 27 (0x61b000002608): array
Node 28 (0x61b000002620): y
Node 29 (0x61b000002638): int(y)
Node 30 (0x61b000002650): array[int(y)]
Node 31 (0x61b000002668): half(array[int(y)])
Node 32 (0x61b000002680): array
Node 33 (0x61b000002698): z
Node 34 (0x61b0000026b0): array[z]
Node 35 (0x61b0000026c8): half(array[z])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): w
Node 38 (0x61b000002710): array[w]
Node 39 (0x61b000002728): half(array[w])
Node 40 (0x61b000002740): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = short(0), float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short(0)
Node 9 (0x61b000002458): short x = short(0)
Node 10 (0x61b000002470): short x = short(0);
Node 11 (0x61b000002488): 1
Node 12 (0x61b0000024a0): ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1)
Node 14 (0x61b0000024d0): ushort y = ushort(1);
Node 15 (0x61b0000024e8): 2
Node 16 (0x61b000002500): int z = 2
Node 17 (0x61b000002518): int z = 2;
Node 18 (0x61b000002530): 3
Node 19 (0x61b000002548): uint w = 3
Node 20 (0x61b000002560): uint w = 3;
Node 21 (0x61b000002578): sk_FragColor
Node 22 (0x61b000002590): array
Node 23 (0x61b0000025a8): x
Node 24 (0x61b0000025c0): int(x)
Node 25 (0x61b0000025d8): array[int(x)]
Node 26 (0x61b0000025f0): half(array[int(x)])
Node 27 (0x61b000002608): array
Node 28 (0x61b000002620): y
Node 29 (0x61b000002638): int(y)
Node 30 (0x61b000002650): array[int(y)]
Node 31 (0x61b000002668): half(array[int(y)])
Node 32 (0x61b000002680): array
Node 33 (0x61b000002698): z
Node 34 (0x61b0000026b0): array[z]
Node 35 (0x61b0000026c8): half(array[z])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): w
Node 38 (0x61b000002710): array[w]
Node 39 (0x61b000002728): half(array[w])
Node 40 (0x61b000002740): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = short(0), float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short(0)
Node 9 (0x61b000002458): short x = short(0)
Node 10 (0x61b000002470): short x = short(0);
Node 11 (0x61b000002488): 1
Node 12 (0x61b0000024a0): ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1)
Node 14 (0x61b0000024d0): ushort y = ushort(1);
Node 15 (0x61b0000024e8): 2
Node 16 (0x61b000002500): int z = 2
Node 17 (0x61b000002518): int z = 2;
Node 18 (0x61b000002530): 3
Node 19 (0x61b000002548): uint w = 3
Node 20 (0x61b000002560): uint w = 3;
Node 21 (0x61b000002578): sk_FragColor
Node 22 (0x61b000002590): array
Node 23 (0x61b0000025a8): x
Node 24 (0x61b0000025c0): int(x)
Node 25 (0x61b0000025d8): array[int(x)]
Node 26 (0x61b0000025f0): half(array[int(x)])
Node 27 (0x61b000002608): array
Node 28 (0x61b000002620): y
Node 29 (0x61b000002638): int(y)
Node 30 (0x61b000002650): array[int(y)]
Node 31 (0x61b000002668): half(array[int(y)])
Node 32 (0x61b000002680): array
Node 33 (0x61b000002698): z
Node 34 (0x61b0000026b0): array[z]
Node 35 (0x61b0000026c8): half(array[z])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): w
Node 38 (0x61b000002710): array[w]
Node 39 (0x61b000002728): half(array[w])
Node 40 (0x61b000002740): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = short(0), float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 3.0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short(0)
Node 9 (0x61b000002458): short x = short(0)
Node 10 (0x61b000002470): short x = short(0);
Node 11 (0x61b000002488): 1
Node 12 (0x61b0000024a0): ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1)
Node 14 (0x61b0000024d0): ushort y = ushort(1);
Node 15 (0x61b0000024e8): 2
Node 16 (0x61b000002500): int z = 2
Node 17 (0x61b000002518): int z = 2;
Node 18 (0x61b000002530): 3
Node 19 (0x61b000002548): uint w = 3
Node 20 (0x61b000002560): uint w = 3;
Node 21 (0x61b000002578): sk_FragColor
Node 22 (0x61b000002590): array
Node 23 (0x61b0000025a8): x
Node 24 (0x61b0000025c0): int(x)
Node 25 (0x61b0000025d8): array[int(x)]
Node 26 (0x61b0000025f0): half(array[int(x)])
Node 27 (0x61b000002608): array
Node 28 (0x61b000002620): y
Node 29 (0x61b000002638): int(y)
Node 30 (0x61b000002650): array[int(y)]
Node 31 (0x61b000002668): half(array[int(y)])
Node 32 (0x61b000002680): array
Node 33 (0x61b000002698): z
Node 34 (0x61b0000026b0): array[z]
Node 35 (0x61b0000026c8): half(array[z])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): w
Node 38 (0x61b000002710): array[w]
Node 39 (0x61b000002728): half(array[w])
Node 40 (0x61b000002740): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = short(0), float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 4.0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short(0)
Node 9 (0x61b000002458): short x = short(0)
Node 10 (0x61b000002470): short x = short(0);
Node 11 (0x61b000002488): 1
Node 12 (0x61b0000024a0): ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1)
Node 14 (0x61b0000024d0): ushort y = ushort(1);
Node 15 (0x61b0000024e8): 2
Node 16 (0x61b000002500): int z = 2
Node 17 (0x61b000002518): int z = 2;
Node 18 (0x61b000002530): 3
Node 19 (0x61b000002548): uint w = 3
Node 20 (0x61b000002560): uint w = 3;
Node 21 (0x61b000002578): sk_FragColor
Node 22 (0x61b000002590): array
Node 23 (0x61b0000025a8): x
Node 24 (0x61b0000025c0): int(x)
Node 25 (0x61b0000025d8): array[int(x)]
Node 26 (0x61b0000025f0): half(array[int(x)])
Node 27 (0x61b000002608): array
Node 28 (0x61b000002620): y
Node 29 (0x61b000002638): int(y)
Node 30 (0x61b000002650): array[int(y)]
Node 31 (0x61b000002668): half(array[int(y)])
Node 32 (0x61b000002680): array
Node 33 (0x61b000002698): z
Node 34 (0x61b0000026b0): array[z]
Node 35 (0x61b0000026c8): half(array[z])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): w
Node 38 (0x61b000002710): array[w]
Node 39 (0x61b000002728): half(array[w])
Node 40 (0x61b000002740): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = short(0), float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX float[4](1.0, 2.0, 3.0, 4.0) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short(0)
Node 9 (0x61b000002458): short x = short(0)
Node 10 (0x61b000002470): short x = short(0);
Node 11 (0x61b000002488): 1
Node 12 (0x61b0000024a0): ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1)
Node 14 (0x61b0000024d0): ushort y = ushort(1);
Node 15 (0x61b0000024e8): 2
Node 16 (0x61b000002500): int z = 2
Node 17 (0x61b000002518): int z = 2;
Node 18 (0x61b000002530): 3
Node 19 (0x61b000002548): uint w = 3
Node 20 (0x61b000002560): uint w = 3;
Node 21 (0x61b000002578): sk_FragColor
Node 22 (0x61b000002590): array
Node 23 (0x61b0000025a8): x
Node 24 (0x61b0000025c0): int(x)
Node 25 (0x61b0000025d8): array[int(x)]
Node 26 (0x61b0000025f0): half(array[int(x)])
Node 27 (0x61b000002608): array
Node 28 (0x61b000002620): y
Node 29 (0x61b000002638): int(y)
Node 30 (0x61b000002650): array[int(y)]
Node 31 (0x61b000002668): half(array[int(y)])
Node 32 (0x61b000002680): array
Node 33 (0x61b000002698): z
Node 34 (0x61b0000026b0): array[z]
Node 35 (0x61b0000026c8): half(array[z])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): w
Node 38 (0x61b000002710): array[w]
Node 39 (0x61b000002728): half(array[w])
Node 40 (0x61b000002740): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = short(0), float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0) 
simplifying vardecl
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short(0)
Node 9 (0x61b000002458): short x = short(0)
Node 10 (0x61b000002470): short x = short(0);
Node 11 (0x61b000002488): 1
Node 12 (0x61b0000024a0): ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1)
Node 14 (0x61b0000024d0): ushort y = ushort(1);
Node 15 (0x61b0000024e8): 2
Node 16 (0x61b000002500): int z = 2
Node 17 (0x61b000002518): int z = 2;
Node 18 (0x61b000002530): 3
Node 19 (0x61b000002548): uint w = 3
Node 20 (0x61b000002560): uint w = 3;
Node 21 (0x61b000002578): sk_FragColor
Node 22 (0x61b000002590): array
Node 23 (0x61b0000025a8): x
Node 24 (0x61b0000025c0): int(x)
Node 25 (0x61b0000025d8): array[int(x)]
Node 26 (0x61b0000025f0): half(array[int(x)])
Node 27 (0x61b000002608): array
Node 28 (0x61b000002620): y
Node 29 (0x61b000002638): int(y)
Node 30 (0x61b000002650): array[int(y)]
Node 31 (0x61b000002668): half(array[int(y)])
Node 32 (0x61b000002680): array
Node 33 (0x61b000002698): z
Node 34 (0x61b0000026b0): array[z]
Node 35 (0x61b0000026c8): half(array[z])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): w
Node 38 (0x61b000002710): array[w]
Node 39 (0x61b000002728): half(array[w])
Node 40 (0x61b000002740): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = short(0), float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST float array = float[4](1.0, 2.0, 3.0, 4.0); 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short(0)
Node 9 (0x61b000002458): short x = short(0)
Node 10 (0x61b000002470): short x = short(0);
Node 11 (0x61b000002488): 1
Node 12 (0x61b0000024a0): ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1)
Node 14 (0x61b0000024d0): ushort y = ushort(1);
Node 15 (0x61b0000024e8): 2
Node 16 (0x61b000002500): int z = 2
Node 17 (0x61b000002518): int z = 2;
Node 18 (0x61b000002530): 3
Node 19 (0x61b000002548): uint w = 3
Node 20 (0x61b000002560): uint w = 3;
Node 21 (0x61b000002578): sk_FragColor
Node 22 (0x61b000002590): array
Node 23 (0x61b0000025a8): x
Node 24 (0x61b0000025c0): int(x)
Node 25 (0x61b0000025d8): array[int(x)]
Node 26 (0x61b0000025f0): half(array[int(x)])
Node 27 (0x61b000002608): array
Node 28 (0x61b000002620): y
Node 29 (0x61b000002638): int(y)
Node 30 (0x61b000002650): array[int(y)]
Node 31 (0x61b000002668): half(array[int(y)])
Node 32 (0x61b000002680): array
Node 33 (0x61b000002698): z
Node 34 (0x61b0000026b0): array[z]
Node 35 (0x61b0000026c8): half(array[z])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): w
Node 38 (0x61b000002710): array[w]
Node 39 (0x61b000002728): half(array[w])
Node 40 (0x61b000002740): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = short(0), float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short(0)
Node 9 (0x61b000002458): short x = short(0)
Node 10 (0x61b000002470): short x = short(0);
Node 11 (0x61b000002488): 1
Node 12 (0x61b0000024a0): ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1)
Node 14 (0x61b0000024d0): ushort y = ushort(1);
Node 15 (0x61b0000024e8): 2
Node 16 (0x61b000002500): int z = 2
Node 17 (0x61b000002518): int z = 2;
Node 18 (0x61b000002530): 3
Node 19 (0x61b000002548): uint w = 3
Node 20 (0x61b000002560): uint w = 3;
Node 21 (0x61b000002578): sk_FragColor
Node 22 (0x61b000002590): array
Node 23 (0x61b0000025a8): x
Node 24 (0x61b0000025c0): int(x)
Node 25 (0x61b0000025d8): array[int(x)]
Node 26 (0x61b0000025f0): half(array[int(x)])
Node 27 (0x61b000002608): array
Node 28 (0x61b000002620): y
Node 29 (0x61b000002638): int(y)
Node 30 (0x61b000002650): array[int(y)]
Node 31 (0x61b000002668): half(array[int(y)])
Node 32 (0x61b000002680): array
Node 33 (0x61b000002698): z
Node 34 (0x61b0000026b0): array[z]
Node 35 (0x61b0000026c8): half(array[z])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): w
Node 38 (0x61b000002710): array[w]
Node 39 (0x61b000002728): half(array[w])
Node 40 (0x61b000002740): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = short(0), float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX short(0) 
optimized to 0 
coerced to 0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort(1)
Node 12 (0x61b0000024a0): ushort y = ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1);
Node 14 (0x61b0000024d0): 2
Node 15 (0x61b0000024e8): int z = 2
Node 16 (0x61b000002500): int z = 2;
Node 17 (0x61b000002518): 3
Node 18 (0x61b000002530): uint w = 3
Node 19 (0x61b000002548): uint w = 3;
Node 20 (0x61b000002560): sk_FragColor
Node 21 (0x61b000002578): array
Node 22 (0x61b000002590): x
Node 23 (0x61b0000025a8): int(x)
Node 24 (0x61b0000025c0): array[int(x)]
Node 25 (0x61b0000025d8): half(array[int(x)])
Node 26 (0x61b0000025f0): array
Node 27 (0x61b000002608): y
Node 28 (0x61b000002620): int(y)
Node 29 (0x61b000002638): array[int(y)]
Node 30 (0x61b000002650): half(array[int(y)])
Node 31 (0x61b000002668): array
Node 32 (0x61b000002680): z
Node 33 (0x61b000002698): array[z]
Node 34 (0x61b0000026b0): half(array[z])
Node 35 (0x61b0000026c8): array
Node 36 (0x61b0000026e0): w
Node 37 (0x61b0000026f8): array[w]
Node 38 (0x61b000002710): half(array[w])
Node 39 (0x61b000002728): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST short x = 0 
simplifying vardecl
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort(1)
Node 12 (0x61b0000024a0): ushort y = ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1);
Node 14 (0x61b0000024d0): 2
Node 15 (0x61b0000024e8): int z = 2
Node 16 (0x61b000002500): int z = 2;
Node 17 (0x61b000002518): 3
Node 18 (0x61b000002530): uint w = 3
Node 19 (0x61b000002548): uint w = 3;
Node 20 (0x61b000002560): sk_FragColor
Node 21 (0x61b000002578): array
Node 22 (0x61b000002590): x
Node 23 (0x61b0000025a8): int(x)
Node 24 (0x61b0000025c0): array[int(x)]
Node 25 (0x61b0000025d8): half(array[int(x)])
Node 26 (0x61b0000025f0): array
Node 27 (0x61b000002608): y
Node 28 (0x61b000002620): int(y)
Node 29 (0x61b000002638): array[int(y)]
Node 30 (0x61b000002650): half(array[int(y)])
Node 31 (0x61b000002668): array
Node 32 (0x61b000002680): z
Node 33 (0x61b000002698): array[z]
Node 34 (0x61b0000026b0): half(array[z])
Node 35 (0x61b0000026c8): array
Node 36 (0x61b0000026e0): w
Node 37 (0x61b0000026f8): array[w]
Node 38 (0x61b000002710): half(array[w])
Node 39 (0x61b000002728): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST short x = 0; 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort(1)
Node 12 (0x61b0000024a0): ushort y = ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1);
Node 14 (0x61b0000024d0): 2
Node 15 (0x61b0000024e8): int z = 2
Node 16 (0x61b000002500): int z = 2;
Node 17 (0x61b000002518): 3
Node 18 (0x61b000002530): uint w = 3
Node 19 (0x61b000002548): uint w = 3;
Node 20 (0x61b000002560): sk_FragColor
Node 21 (0x61b000002578): array
Node 22 (0x61b000002590): x
Node 23 (0x61b0000025a8): int(x)
Node 24 (0x61b0000025c0): array[int(x)]
Node 25 (0x61b0000025d8): half(array[int(x)])
Node 26 (0x61b0000025f0): array
Node 27 (0x61b000002608): y
Node 28 (0x61b000002620): int(y)
Node 29 (0x61b000002638): array[int(y)]
Node 30 (0x61b000002650): half(array[int(y)])
Node 31 (0x61b000002668): array
Node 32 (0x61b000002680): z
Node 33 (0x61b000002698): array[z]
Node 34 (0x61b0000026b0): half(array[z])
Node 35 (0x61b0000026c8): array
Node 36 (0x61b0000026e0): w
Node 37 (0x61b0000026f8): array[w]
Node 38 (0x61b000002710): half(array[w])
Node 39 (0x61b000002728): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort(1)
Node 12 (0x61b0000024a0): ushort y = ushort(1)
Node 13 (0x61b0000024b8): ushort y = ushort(1);
Node 14 (0x61b0000024d0): 2
Node 15 (0x61b0000024e8): int z = 2
Node 16 (0x61b000002500): int z = 2;
Node 17 (0x61b000002518): 3
Node 18 (0x61b000002530): uint w = 3
Node 19 (0x61b000002548): uint w = 3;
Node 20 (0x61b000002560): sk_FragColor
Node 21 (0x61b000002578): array
Node 22 (0x61b000002590): x
Node 23 (0x61b0000025a8): int(x)
Node 24 (0x61b0000025c0): array[int(x)]
Node 25 (0x61b0000025d8): half(array[int(x)])
Node 26 (0x61b0000025f0): array
Node 27 (0x61b000002608): y
Node 28 (0x61b000002620): int(y)
Node 29 (0x61b000002638): array[int(y)]
Node 30 (0x61b000002650): half(array[int(y)])
Node 31 (0x61b000002668): array
Node 32 (0x61b000002680): z
Node 33 (0x61b000002698): array[z]
Node 34 (0x61b0000026b0): half(array[z])
Node 35 (0x61b0000026c8): array
Node 36 (0x61b0000026e0): w
Node 37 (0x61b0000026f8): array[w]
Node 38 (0x61b000002710): half(array[w])
Node 39 (0x61b000002728): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = ushort(1), short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX ushort(1) 
optimized to 1 
coerced to 1 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST ushort y = 1 
simplifying vardecl
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST ushort y = 1; 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 2 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST int z = 2 
simplifying vardecl
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST int z = 2; 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 3 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST uint w = 3 
simplifying vardecl
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST uint w = 3; 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): array
Node 21 (0x61b000002578): x
Node 22 (0x61b000002590): int(x)
Node 23 (0x61b0000025a8): array[int(x)]
Node 24 (0x61b0000025c0): half(array[int(x)])
Node 25 (0x61b0000025d8): array
Node 26 (0x61b0000025f0): y
Node 27 (0x61b000002608): int(y)
Node 28 (0x61b000002620): array[int(y)]
Node 29 (0x61b000002638): half(array[int(y)])
Node 30 (0x61b000002650): array
Node 31 (0x61b000002668): z
Node 32 (0x61b000002680): array[z]
Node 33 (0x61b000002698): half(array[z])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): w
Node 36 (0x61b0000026e0): array[w]
Node 37 (0x61b0000026f8): half(array[w])
Node 38 (0x61b000002710): half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 39 (0x61b000002728): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 40 (0x61b000002740): (sk_FragColor = half4(half(array[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX array 
optimized to float[4](1.0, 2.0, 3.0, 4.0) 
coerced to float[4](1.0, 2.0, 3.0, 4.0) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): x
Node 26 (0x61b0000025f0): int(x)
Node 27 (0x61b000002608): float[4](1.0, 2.0, 3.0, 4.0)[int(x)]
Node 28 (0x61b000002620): half(float[4](1.0, 2.0, 3.0, 4.0)[int(x)])
Node 29 (0x61b000002638): array
Node 30 (0x61b000002650): y
Node 31 (0x61b000002668): int(y)
Node 32 (0x61b000002680): array[int(y)]
Node 33 (0x61b000002698): half(array[int(y)])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): z
Node 36 (0x61b0000026e0): array[z]
Node 37 (0x61b0000026f8): half(array[z])
Node 38 (0x61b000002710): array
Node 39 (0x61b000002728): w
Node 40 (0x61b000002740): array[w]
Node 41 (0x61b000002758): half(array[w])
Node 42 (0x61b000002770): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[int(x)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 43 (0x61b000002788): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[int(x)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to 0 
coerced to 0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): int(0)
Node 27 (0x61b000002608): float[4](1.0, 2.0, 3.0, 4.0)[int(0)]
Node 28 (0x61b000002620): half(float[4](1.0, 2.0, 3.0, 4.0)[int(0)])
Node 29 (0x61b000002638): array
Node 30 (0x61b000002650): y
Node 31 (0x61b000002668): int(y)
Node 32 (0x61b000002680): array[int(y)]
Node 33 (0x61b000002698): half(array[int(y)])
Node 34 (0x61b0000026b0): array
Node 35 (0x61b0000026c8): z
Node 36 (0x61b0000026e0): array[z]
Node 37 (0x61b0000026f8): half(array[z])
Node 38 (0x61b000002710): array
Node 39 (0x61b000002728): w
Node 40 (0x61b000002740): array[w]
Node 41 (0x61b000002758): half(array[w])
Node 42 (0x61b000002770): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[int(0)]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 43 (0x61b000002788): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[int(0)]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[int(0)]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX int(0) 
optimized to 0 
coerced to 0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): array
Node 29 (0x61b000002638): y
Node 30 (0x61b000002650): int(y)
Node 31 (0x61b000002668): array[int(y)]
Node 32 (0x61b000002680): half(array[int(y)])
Node 33 (0x61b000002698): array
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): array[z]
Node 36 (0x61b0000026e0): half(array[z])
Node 37 (0x61b0000026f8): array
Node 38 (0x61b000002710): w
Node 39 (0x61b000002728): array[w]
Node 40 (0x61b000002740): half(array[w])
Node 41 (0x61b000002758): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 43 (0x61b000002788): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX float[4](1.0, 2.0, 3.0, 4.0)[0] 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): array
Node 29 (0x61b000002638): y
Node 30 (0x61b000002650): int(y)
Node 31 (0x61b000002668): array[int(y)]
Node 32 (0x61b000002680): half(array[int(y)])
Node 33 (0x61b000002698): array
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): array[z]
Node 36 (0x61b0000026e0): half(array[z])
Node 37 (0x61b0000026f8): array
Node 38 (0x61b000002710): w
Node 39 (0x61b000002728): array[w]
Node 40 (0x61b000002740): half(array[w])
Node 41 (0x61b000002758): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 43 (0x61b000002788): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX half(float[4](1.0, 2.0, 3.0, 4.0)[0]) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): array
Node 29 (0x61b000002638): y
Node 30 (0x61b000002650): int(y)
Node 31 (0x61b000002668): array[int(y)]
Node 32 (0x61b000002680): half(array[int(y)])
Node 33 (0x61b000002698): array
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): array[z]
Node 36 (0x61b0000026e0): half(array[z])
Node 37 (0x61b0000026f8): array
Node 38 (0x61b000002710): w
Node 39 (0x61b000002728): array[w]
Node 40 (0x61b000002740): half(array[w])
Node 41 (0x61b000002758): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(array[int(y)]), half(array[z]), half(array[w]))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(array[int(y)]), half(array[z]), half(array[w])))
Node 43 (0x61b000002788): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(array[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX array 
optimized to float[4](1.0, 2.0, 3.0, 4.0) 
coerced to float[4](1.0, 2.0, 3.0, 4.0) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): y
Node 34 (0x61b0000026b0): int(y)
Node 35 (0x61b0000026c8): float[4](1.0, 2.0, 3.0, 4.0)[int(y)]
Node 36 (0x61b0000026e0): half(float[4](1.0, 2.0, 3.0, 4.0)[int(y)])
Node 37 (0x61b0000026f8): array
Node 38 (0x61b000002710): z
Node 39 (0x61b000002728): array[z]
Node 40 (0x61b000002740): half(array[z])
Node 41 (0x61b000002758): array
Node 42 (0x61b000002770): w
Node 43 (0x61b000002788): array[w]
Node 44 (0x61b0000027a0): half(array[w])
Node 45 (0x61b0000027b8): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[int(y)]), half(array[z]), half(array[w]))
Node 46 (0x61b0000027d0): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[int(y)]), half(array[z]), half(array[w])))
Node 47 (0x61b0000027e8): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[int(y)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX y 
optimized to 1 
coerced to 1 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): int(1)
Node 35 (0x61b0000026c8): float[4](1.0, 2.0, 3.0, 4.0)[int(1)]
Node 36 (0x61b0000026e0): half(float[4](1.0, 2.0, 3.0, 4.0)[int(1)])
Node 37 (0x61b0000026f8): array
Node 38 (0x61b000002710): z
Node 39 (0x61b000002728): array[z]
Node 40 (0x61b000002740): half(array[z])
Node 41 (0x61b000002758): array
Node 42 (0x61b000002770): w
Node 43 (0x61b000002788): array[w]
Node 44 (0x61b0000027a0): half(array[w])
Node 45 (0x61b0000027b8): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[int(1)]), half(array[z]), half(array[w]))
Node 46 (0x61b0000027d0): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[int(1)]), half(array[z]), half(array[w])))
Node 47 (0x61b0000027e8): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[int(1)]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX int(1) 
optimized to 1 
coerced to 1 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): z
Node 38 (0x61b000002710): array[z]
Node 39 (0x61b000002728): half(array[z])
Node 40 (0x61b000002740): array
Node 41 (0x61b000002758): w
Node 42 (0x61b000002770): array[w]
Node 43 (0x61b000002788): half(array[w])
Node 44 (0x61b0000027a0): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(array[z]), half(array[w]))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(array[z]), half(array[w])))
Node 46 (0x61b0000027d0): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX float[4](1.0, 2.0, 3.0, 4.0)[1] 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): z
Node 38 (0x61b000002710): array[z]
Node 39 (0x61b000002728): half(array[z])
Node 40 (0x61b000002740): array
Node 41 (0x61b000002758): w
Node 42 (0x61b000002770): array[w]
Node 43 (0x61b000002788): half(array[w])
Node 44 (0x61b0000027a0): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(array[z]), half(array[w]))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(array[z]), half(array[w])))
Node 46 (0x61b0000027d0): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX half(float[4](1.0, 2.0, 3.0, 4.0)[1]) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): array
Node 37 (0x61b0000026f8): z
Node 38 (0x61b000002710): array[z]
Node 39 (0x61b000002728): half(array[z])
Node 40 (0x61b000002740): array
Node 41 (0x61b000002758): w
Node 42 (0x61b000002770): array[w]
Node 43 (0x61b000002788): half(array[w])
Node 44 (0x61b0000027a0): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(array[z]), half(array[w]))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(array[z]), half(array[w])))
Node 46 (0x61b0000027d0): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(array[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX array 
optimized to float[4](1.0, 2.0, 3.0, 4.0) 
coerced to float[4](1.0, 2.0, 3.0, 4.0) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): z
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[z]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[z])
Node 44 (0x61b0000027a0): array
Node 45 (0x61b0000027b8): w
Node 46 (0x61b0000027d0): array[w]
Node 47 (0x61b0000027e8): half(array[w])
Node 48 (0x61b000002800): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[z]), half(array[w]))
Node 49 (0x61b000002818): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[z]), half(array[w])))
Node 50 (0x61b000002830): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[z]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX z 
optimized to 2 
coerced to 2 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): array
Node 45 (0x61b0000027b8): w
Node 46 (0x61b0000027d0): array[w]
Node 47 (0x61b0000027e8): half(array[w])
Node 48 (0x61b000002800): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(array[w]))
Node 49 (0x61b000002818): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(array[w])))
Node 50 (0x61b000002830): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX float[4](1.0, 2.0, 3.0, 4.0)[2] 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): array
Node 45 (0x61b0000027b8): w
Node 46 (0x61b0000027d0): array[w]
Node 47 (0x61b0000027e8): half(array[w])
Node 48 (0x61b000002800): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(array[w]))
Node 49 (0x61b000002818): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(array[w])))
Node 50 (0x61b000002830): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX half(float[4](1.0, 2.0, 3.0, 4.0)[2]) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): array
Node 45 (0x61b0000027b8): w
Node 46 (0x61b0000027d0): array[w]
Node 47 (0x61b0000027e8): half(array[w])
Node 48 (0x61b000002800): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(array[w]))
Node 49 (0x61b000002818): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(array[w])))
Node 50 (0x61b000002830): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(array[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX array 
optimized to float[4](1.0, 2.0, 3.0, 4.0) 
coerced to float[4](1.0, 2.0, 3.0, 4.0) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): w
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[w]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[w])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[w]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[w])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[w])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX w 
optimized to 3 
coerced to 3 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX float[4](1.0, 2.0, 3.0, 4.0)[3] 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX half(float[4](1.0, 2.0, 3.0, 4.0)[3]) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))); 
simplifying expr
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 3.0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
N=================================================================
==77281==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013cd0 at pc 0x000108b82c05 bp 0x7ffee7192f00 sp 0x7ffee7192ef8
READ of size 8 at 0x60d000013cd0 thread T0
    #0 0x108b82c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x108b819a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x108b80fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x108c26d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x108c35657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x108c326e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x108a6e108 in main SkSLMain.cpp:258
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013cd0 is located 128 bytes inside of 136-byte region [0x60d000013c50,0x60d000013cd8)
freed by thread T0 here:
    #0 0x10a7a0c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x108f0a8dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x108d47e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x108c2208d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x108c6d97d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x108c21a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x108c1d4a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x108c27012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x108c35657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x108c326e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x108a6e108 in main SkSLMain.cpp:258
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10a7a084d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x108de2fdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x108dd59a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x108dbc116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x108dba17e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x108dcb0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x108dbba03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x108dfe8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x108e63663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x108c31e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x108a6e108 in main SkSLMain.cpp:258
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
  0x1c1a000027b0: 00 00 00 00 00 fa fa fa fa fa fa fa fa fa 00 00
  0x1c1a000027c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa 00 00 00 00 00 00 00 00
  0x1c1a000027e0: 00 00 00 00 00 00 00 00 00 fa fa fa fa fa fa fa
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
==77281==ABORTING
ode 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX 4.0 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify EX float[4](1.0, 2.0, 3.0, 4.0) 
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): 2.0
Node 2 (0x61b0000023b0): 3.0
Node 3 (0x61b0000023c8): 4.0
Node 4 (0x61b0000023e0): float[4](1.0, 2.0, 3.0, 4.0)
Node 5 (0x61b0000023f8): float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0)
Node 6 (0x61b000002410): float array = float[4](1.0, 2.0, 3.0, 4.0);
Node 7 (0x61b000002428): 0
Node 8 (0x61b000002440): short x = 0
Node 9 (0x61b000002458): short x = 0;
Node 10 (0x61b000002470): 1
Node 11 (0x61b000002488): ushort y = 1
Node 12 (0x61b0000024a0): ushort y = 1;
Node 13 (0x61b0000024b8): 2
Node 14 (0x61b0000024d0): int z = 2
Node 15 (0x61b0000024e8): int z = 2;
Node 16 (0x61b000002500): 3
Node 17 (0x61b000002518): uint w = 3
Node 18 (0x61b000002530): uint w = 3;
Node 19 (0x61b000002548): sk_FragColor
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): 3.0
Node 23 (0x61b0000025a8): 4.0
Node 24 (0x61b0000025c0): float[4](1.0, 2.0, 3.0, 4.0)
Node 25 (0x61b0000025d8): 0
Node 26 (0x61b0000025f0): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 27 (0x61b000002608): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): 3.0
Node 31 (0x61b000002668): 4.0
Node 32 (0x61b000002680): float[4](1.0, 2.0, 3.0, 4.0)
Node 33 (0x61b000002698): 1
Node 34 (0x61b0000026b0): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 35 (0x61b0000026c8): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): 2.0
Node 38 (0x61b000002710): 3.0
Node 39 (0x61b000002728): 4.0
Node 40 (0x61b000002740): float[4](1.0, 2.0, 3.0, 4.0)
Node 41 (0x61b000002758): 2
Node 42 (0x61b000002770): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 43 (0x61b000002788): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 44 (0x61b0000027a0): 1.0
Node 45 (0x61b0000027b8): 2.0
Node 46 (0x61b0000027d0): 3.0
Node 47 (0x61b0000027e8): 4.0
Node 48 (0x61b000002800): float[4](1.0, 2.0, 3.0, 4.0)
Node 49 (0x61b000002818): 3
Node 50 (0x61b000002830): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 51 (0x61b000002848): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 52 (0x61b000002860): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 54 (0x61b000002890): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0, float[4] array = float[4](1.0, 2.0, 3.0, 4.0)]
Entrances: [0]
Exits: []

about to simplify ST float[4] array[4] = float[4](1.0, 2.0, 3.0, 4.0) 
simplifying vardecl
Block 0
-------
Before: [uint w = <undefined>, int z = <undefined>, ushort y = <undefined>, short x = <undefined>, float[4] array = <undefined>]
Entrances: []
Node 0 (0x61b000002380): ;
Node 1 (0x61b000002398): float ;
Node 2 (0x61b0000023b0): 0
Node 3 (0x61b0000023c8): short x = 0
Node 4 (0x61b0000023e0): short x = 0;
Node 5 (0x61b0000023f8): 1
Node 6 (0x61b000002410): ushort y = 1
Node 7 (0x61b000002428): ushort y = 1;
Node 8 (0x61b000002440): 2
Node 9 (0x61b000002458): int z = 2
Node 10 (0x61b000002470): int z = 2;
Node 11 (0x61b000002488): 3
Node 12 (0x61b0000024a0): uint w = 3
Node 13 (0x61b0000024b8): uint w = 3;
Node 14 (0x61b0000024d0): sk_FragColor
Node 15 (0x61b0000024e8): 1.0
Node 16 (0x61b000002500): 2.0
Node 17 (0x61b000002518): 3.0
Node 18 (0x61b000002530): 4.0
Node 19 (0x61b000002548): float[4](1.0, 2.0, 3.0, 4.0)
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): float[4](1.0, 2.0, 3.0, 4.0)[0]
Node 22 (0x61b000002590): half(float[4](1.0, 2.0, 3.0, 4.0)[0])
Node 23 (0x61b0000025a8): 1.0
Node 24 (0x61b0000025c0): 2.0
Node 25 (0x61b0000025d8): 3.0
Node 26 (0x61b0000025f0): 4.0
Node 27 (0x61b000002608): float[4](1.0, 2.0, 3.0, 4.0)
Node 28 (0x61b000002620): 1
Node 29 (0x61b000002638): float[4](1.0, 2.0, 3.0, 4.0)[1]
Node 30 (0x61b000002650): half(float[4](1.0, 2.0, 3.0, 4.0)[1])
Node 31 (0x61b000002668): 1.0
Node 32 (0x61b000002680): 2.0
Node 33 (0x61b000002698): 3.0
Node 34 (0x61b0000026b0): 4.0
Node 35 (0x61b0000026c8): float[4](1.0, 2.0, 3.0, 4.0)
Node 36 (0x61b0000026e0): 2
Node 37 (0x61b0000026f8): float[4](1.0, 2.0, 3.0, 4.0)[2]
Node 38 (0x61b000002710): half(float[4](1.0, 2.0, 3.0, 4.0)[2])
Node 39 (0x61b000002728): 1.0
Node 40 (0x61b000002740): 2.0
Node 41 (0x61b000002758): 3.0
Node 42 (0x61b000002770): 4.0
Node 43 (0x61b000002788): float[4](1.0, 2.0, 3.0, 4.0)
Node 44 (0x61b0000027a0): 3
Node 45 (0x61b0000027b8): float[4](1.0, 2.0, 3.0, 4.0)[3]
Node 46 (0x61b0000027d0): half(float[4](1.0, 2.0, 3.0, 4.0)[3])
Node 47 (0x61b0000027e8): half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3]))
Node 48 (0x61b000002800): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])))
Node 49 (0x61b000002818): (sk_FragColor = half4(half(float[4](1.0, 2.0, 3.0, 4.0)[0]), half(float[4](1.0, 2.0, 3.0, 4.0)[1]), half(float[4](1.0, 2.0, 3.0, 4.0)[2]), half(float[4](1.0, 2.0, 3.0, 4.0)[3])));
Exits: [1]

Block 1
-------
Before: [uint w = 3, int z = 2, ushort y = 1, short x = 0
