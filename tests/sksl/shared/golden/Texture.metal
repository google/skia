### Compilation failed:

Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX one 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sample(one, 0.0) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float4(sample(one, 0.0)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 a = float4(sample(one, 0.0)) 
simplifying vardecl
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 a = float4(sample(one, 0.0)); 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX two 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float2(0.0) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sample(two, float2(0.0)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float4(sample(two, float2(0.0))) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 b = float4(sample(two, float2(0.0))) 
simplifying vardecl
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 b = float4(sample(two, float2(0.0))); 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX one 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float2(0.0) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sample(one, float2(0.0)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float4(sample(one, float2(0.0))) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 c = float4(sample(one, float2(0.0))) 
simplifying vardecl
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 c = float4(sample(one, float2(0.0))); 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX two 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float3(0.0) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sample(two, float3(0.0)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float4(sample(two, float3(0.0))) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 d = float4(sample(two, float3(0.0))) 
simplifying vardecl
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 d = float4(sample(two, float3(0.0))); 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX a 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX a.x 
optimizing swiz 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half(a.x) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX b 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX b.x 
optimizing swiz 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half(b.x) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX c 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX c.x 
optimizing swiz 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half(c.x) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX d 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX d.x 
optimizing swiz 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half(d.x) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half4(half(a.x), half(b.x), half(c.x), half(d.x)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x))) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002380): one
Node 1 (0x61b000002398): 0.0
Node 2 (0x61b0000023b0): sample(one, 0.0)
Node 3 (0x61b0000023c8): float4(sample(one, 0.0))
Node 4 (0x61b0000023e0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b0000023f8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002410): two
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): float2(0.0)
Node 9 (0x61b000002458): sample(two, float2(0.0))
Node 10 (0x61b000002470): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002488): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b0000024a0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b0000024b8): one
Node 14 (0x61b0000024d0): 0.0
Node 15 (0x61b0000024e8): float2(0.0)
Node 16 (0x61b000002500): sample(one, float2(0.0))
Node 17 (0x61b000002518): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002530): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002548): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002560): two
Node 21 (0x61b000002578): 0.0
Node 22 (0x61b000002590): float3(0.0)
Node 23 (0x61b0000025a8): sample(two, float3(0.0))
Node 24 (0x61b0000025c0): float4(sample(two, float3(0.0)))
Node 25 (0x61b0000025d8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b0000025f0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002608): sk_FragColor
Node 28 (0x61b000002620): a
Node 29 (0x61b000002638): a.x
Node 30 (0x61b000002650): half(a.x)
Node 31 (0x61b000002668): b
Node 32 (0x61b000002680): b.x
Node 33 (0x61b000002698): half(b.x)
Node 34 (0x61b0000026b0): c
Node 35 (0x61b0000026c8): c.x
Node 36 (0x61b0000026e0): half(c.x)
Node 37 (0x61b0000026f8): d
Node 38 (0x61b000002710): d.x
Node 39 (0x61b000002728): half(d.x)
Node 40 (0x61b000002740): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002758): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002770): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x))); 
simplifying expr
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX one 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sample(one, 0.0) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float4(sample(one, 0.0)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 a = float4(sample(one, 0.0)) 
simplifying vardecl
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 a = float4(sample(one, 0.0)); 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX two 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float2(0.0) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sample(two, float2(0.0)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float4(sample(two, float2(0.0))) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 b = float4(sample(two, float2(0.0))) 
simplifying vardecl
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 b = float4(sample(two, float2(0.0))); 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX one 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float2(0.0) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sample(one, float2(0.0)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float4(sample(one, float2(0.0))) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 c = float4(sample(one, float2(0.0))) 
simplifying vardecl
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 c = float4(sample(one, float2(0.0))); 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX two 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float3(0.0) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sample(two, float3(0.0)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX float4(sample(two, float3(0.0))) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 d = float4(sample(two, float3(0.0))) 
simplifying vardecl
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST float4 d = float4(sample(two, float3(0.0))); 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX a 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX a.x 
optimizing swiz 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half(a.x) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX b 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX b.x 
optimizing swiz 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half(b.x) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX c 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX c.x 
optimizing swiz 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half(c.x) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX d 
optimizing varref 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX d.x 
optimizing swiz 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half(d.x) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX half4(half(a.x), half(b.x), half(c.x), half(d.x)) 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x))) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [float4 d = <undefined>, float4 c = <undefined>, float4 b = <undefined>, float4 a = <undefined>]
Entrances: []
Node 0 (0x61b000002a80): one
Node 1 (0x61b000002a98): 0.0
Node 2 (0x61b000002ab0): sample(one, 0.0)
Node 3 (0x61b000002ac8): float4(sample(one, 0.0))
Node 4 (0x61b000002ae0): float4 a = float4(sample(one, 0.0))
Node 5 (0x61b000002af8): float4 a = float4(sample(one, 0.0));
Node 6 (0x61b000002b10): two
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): float2(0.0)
Node 9 (0x61b000002b58): sample(two, float2(0.0))
Node 10 (0x61b000002b70): float4(sample(two, float2(0.0)))
Node 11 (0x61b000002b88): float4 b = float4(sample(two, float2(0.0)))
Node 12 (0x61b000002ba0): float4 b = float4(sample(two, float2(0.0)));
Node 13 (0x61b000002bb8): one
Node 14 (0x61b000002bd0): 0.0
Node 15 (0x61b000002be8): float2(0.0)
Node 16 (0x61b000002c00): sample(one, float2(0.0))
Node 17 (0x61b000002c18): float4(sample(one, float2(0.0)))
Node 18 (0x61b000002c30): float4 c = float4(sample(one, float2(0.0)))
Node 19 (0x61b000002c48): float4 c = float4(sample(one, float2(0.0)));
Node 20 (0x61b000002c60): two
Node 21 (0x61b000002c78): 0.0
Node 22 (0x61b000002c90): float3(0.0)
Node 23 (0x61b000002ca8): sample(two, float3(0.0))
Node 24 (0x61b000002cc0): float4(sample(two, float3(0.0)))
Node 25 (0x61b000002cd8): float4 d = float4(sample(two, float3(0.0)))
Node 26 (0x61b000002cf0): float4 d = float4(sample(two, float3(0.0)));
Node 27 (0x61b000002d08): sk_FragColor
Node 28 (0x61b000002d20): a
Node 29 (0x61b000002d38): a.x
Node 30 (0x61b000002d50): half(a.x)
Node 31 (0x61b000002d68): b
Node 32 (0x61b000002d80): b.x
Node 33 (0x61b000002d98): half(b.x)
Node 34 (0x61b000002db0): c
Node 35 (0x61b000002dc8): c.x
Node 36 (0x61b000002de0): half(c.x)
Node 37 (0x61b000002df8): d
Node 38 (0x61b000002e10): d.x
Node 39 (0x61b000002e28): half(d.x)
Node 40 (0x61b000002e40): half4(half(a.x), half(b.x), half(c.x), half(d.x))
Node 41 (0x61b000002e58): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)))
Node 42 (0x61b000002e70): (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x)));
Exits: [1]

Block 1
-------
Before: [float4 d = float4(sample(two, float3(0.0))), float4 c = float4(sample(one, float2(0.0))), float4 b = float4(sample(two, float2(0.0))), float4 a = float4(sample(one, 0.0))]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x))); 
simplifying expr
