### Compilation failed:

Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX float2x4(1.0) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float2x4 x = float2x4(1.0) 
simplifying vardecl
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float2x4 x = float2x4(1.0); 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX float2(2.0, 2.0) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)) 
simplifying vardecl
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)); 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): x
Node 15 (0x61b0000024e8): y
Node 16 (0x61b000002500): (x * y)
Node 17 (0x61b000002518): float3x4 z = (x * y)
Node 18 (0x61b000002530): float3x4 z = (x * y);
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): float3x3(1.0)
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float3(2.0)
Node 23 (0x61b0000025a8): (float3x3(1.0) * float3(2.0))
Node 24 (0x61b0000025c0): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 26 (0x61b0000025f0): 2.0
Node 27 (0x61b000002608): float3(2.0)
Node 28 (0x61b000002620): 1.0
Node 29 (0x61b000002638): float3x3(1.0)
Node 30 (0x61b000002650): (float3(2.0) * float3x3(1.0))
Node 31 (0x61b000002668): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 33 (0x61b000002698): sk_FragColor
Node 34 (0x61b0000026b0): z
Node 35 (0x61b0000026c8): 0
Node 36 (0x61b0000026e0): z[0]
Node 37 (0x61b0000026f8): z[0].x
Node 38 (0x61b000002710): half(z[0].x)
Node 39 (0x61b000002728): v1
Node 40 (0x61b000002740): v2
Node 41 (0x61b000002758): (v1 + v2)
Node 42 (0x61b000002770): half3((v1 + v2))
Node 43 (0x61b000002788): half4(half(z[0].x), half3((v1 + v2)))
Node 44 (0x61b0000027a0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (x * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to float2x4(1.0) 
coerced to float2x4(1.0) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): y
Node 17 (0x61b000002518): (float2x4(1.0) * y)
Node 18 (0x61b000002530): float3x4 z = (float2x4(1.0) * y)
Node 19 (0x61b000002548): float3x4 z = (float2x4(1.0) * y);
Node 20 (0x61b000002560): 1.0
Node 21 (0x61b000002578): float3x3(1.0)
Node 22 (0x61b000002590): 2.0
Node 23 (0x61b0000025a8): float3(2.0)
Node 24 (0x61b0000025c0): (float3x3(1.0) * float3(2.0))
Node 25 (0x61b0000025d8): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 26 (0x61b0000025f0): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 27 (0x61b000002608): 2.0
Node 28 (0x61b000002620): float3(2.0)
Node 29 (0x61b000002638): 1.0
Node 30 (0x61b000002650): float3x3(1.0)
Node 31 (0x61b000002668): (float3(2.0) * float3x3(1.0))
Node 32 (0x61b000002680): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 33 (0x61b000002698): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 34 (0x61b0000026b0): sk_FragColor
Node 35 (0x61b0000026c8): z
Node 36 (0x61b0000026e0): 0
Node 37 (0x61b0000026f8): z[0]
Node 38 (0x61b000002710): z[0].x
Node 39 (0x61b000002728): half(z[0].x)
Node 40 (0x61b000002740): v1
Node 41 (0x61b000002758): v2
Node 42 (0x61b000002770): (v1 + v2)
Node 43 (0x61b000002788): half3((v1 + v2))
Node 44 (0x61b0000027a0): half4(half(z[0].x), half3((v1 + v2)))
Node 45 (0x61b0000027b8): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 46 (0x61b0000027d0): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * y), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX y 
optimized to float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)) 
coerced to float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))) 
simplifying vardecl
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))); 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX float3x3(1.0) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX float3(2.0) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX (float3x3(1.0) * float3(2.0)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float3 v1 = (float3x3(1.0) * float3(2.0)) 
simplifying vardecl
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float3 v1 = (float3x3(1.0) * float3(2.0)); 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX float3(2.0) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX float3x3(1.0) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX (float3(2.0) * float3x3(1.0)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float3 v2 = (float3(2.0) * float3x3(1.0)) 
simplifying vardecl
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float3 v2 = (float3(2.0) * float3x3(1.0)); 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX z 
optimizing varref 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX z[0] 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX z[0].x 
optimizing swiz 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX half(z[0].x) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX v1 
optimizing varref 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX v2 
optimizing varref 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX (v1 + v2) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX half3((v1 + v2)) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX half4(half(z[0].x), half3((v1 + v2))) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(half(z[0].x), half3((v1 + v2)))) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node =================================================================
==77668==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013cd0 at pc 0x00010e0bac05 bp 0x7ffee1c5af20 sp 0x7ffee1c5af18
READ of size 8 at 0x60d000013cd0 thread T0
    #0 0x10e0bac04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10e0b99a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x10e0b8fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x10e15ed91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x10e16d657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x10e16a6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x10dfa6108 in main SkSLMain.cpp:258
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013cd0 is located 128 bytes inside of 136-byte region [0x60d000013c50,0x60d000013cd8)
freed by thread T0 here:
    #0 0x10ff63c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10e4428dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x10e27fe01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10e15a08d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x10e1a597d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x10e159a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x10e1554a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x10e15f012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x10e16d657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x10e16a6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x10dfa6108 in main SkSLMain.cpp:258
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10ff6384d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10e31afdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x10e30d9a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x10e2f4116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x10e2f217e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x10e3030e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x10e2f3a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x10e3368cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x10e39b663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x10e169e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x10dfa6108 in main SkSLMain.cpp:258
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
==77668==ABORTING
30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(half(z[0].x), half3((v1 + v2)))); 
simplifying expr
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify EX float2x4(1.0) 
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): 1.0
Node 1 (0x61b000002398): float2x4(1.0)
Node 2 (0x61b0000023b0): float2x4 x = float2x4(1.0)
Node 3 (0x61b0000023c8): float2x4 x = float2x4(1.0);
Node 4 (0x61b0000023e0): 1.0
Node 5 (0x61b0000023f8): 0.0
Node 6 (0x61b000002410): 0.0
Node 7 (0x61b000002428): 1.0
Node 8 (0x61b000002440): 2.0
Node 9 (0x61b000002458): 2.0
Node 10 (0x61b000002470): float2(2.0, 2.0)
Node 11 (0x61b000002488): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 12 (0x61b0000024a0): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 13 (0x61b0000024b8): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): float2x4(1.0)
Node 16 (0x61b000002500): 1.0
Node 17 (0x61b000002518): 0.0
Node 18 (0x61b000002530): 0.0
Node 19 (0x61b000002548): 1.0
Node 20 (0x61b000002560): 2.0
Node 21 (0x61b000002578): 2.0
Node 22 (0x61b000002590): float2(2.0, 2.0)
Node 23 (0x61b0000025a8): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 24 (0x61b0000025c0): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 25 (0x61b0000025d8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 26 (0x61b0000025f0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 27 (0x61b000002608): 1.0
Node 28 (0x61b000002620): float3x3(1.0)
Node 29 (0x61b000002638): 2.0
Node 30 (0x61b000002650): float3(2.0)
Node 31 (0x61b000002668): (float3x3(1.0) * float3(2.0))
Node 32 (0x61b000002680): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 33 (0x61b000002698): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 34 (0x61b0000026b0): 2.0
Node 35 (0x61b0000026c8): float3(2.0)
Node 36 (0x61b0000026e0): 1.0
Node 37 (0x61b0000026f8): float3x3(1.0)
Node 38 (0x61b000002710): (float3(2.0) * float3x3(1.0))
Node 39 (0x61b000002728): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 40 (0x61b000002740): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 41 (0x61b000002758): sk_FragColor
Node 42 (0x61b000002770): z
Node 43 (0x61b000002788): 0
Node 44 (0x61b0000027a0): z[0]
Node 45 (0x61b0000027b8): z[0].x
Node 46 (0x61b0000027d0): half(z[0].x)
Node 47 (0x61b0000027e8): v1
Node 48 (0x61b000002800): v2
Node 49 (0x61b000002818): (v1 + v2)
Node 50 (0x61b000002830): half3((v1 + v2))
Node 51 (0x61b000002848): half4(half(z[0].x), half3((v1 + v2)))
Node 52 (0x61b000002860): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 53 (0x61b000002878): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)), float2x4 x = float2x4(1.0)]
Entrances: [0]
Exits: []

about to simplify ST float2x4 x = float2x4(1.0) 
simplifying vardecl
Block 0
-------
Before: [float3 v2 = <undefined>, float3 v1 = <undefined>, float3x4 z = <undefined>, float3x2 y = <undefined>, float2x4 x = <undefined>]
Entrances: []
Node 0 (0x61b000002380): ;
Node 1 (0x61b000002398): float2x4 ;
Node 2 (0x61b0000023b0): 1.0
Node 3 (0x61b0000023c8): 0.0
Node 4 (0x61b0000023e0): 0.0
Node 5 (0x61b0000023f8): 1.0
Node 6 (0x61b000002410): 2.0
Node 7 (0x61b000002428): 2.0
Node 8 (0x61b000002440): float2(2.0, 2.0)
Node 9 (0x61b000002458): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 10 (0x61b000002470): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 11 (0x61b000002488): float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0));
Node 12 (0x61b0000024a0): 1.0
Node 13 (0x61b0000024b8): float2x4(1.0)
Node 14 (0x61b0000024d0): 1.0
Node 15 (0x61b0000024e8): 0.0
Node 16 (0x61b000002500): 0.0
Node 17 (0x61b000002518): 1.0
Node 18 (0x61b000002530): 2.0
Node 19 (0x61b000002548): 2.0
Node 20 (0x61b000002560): float2(2.0, 2.0)
Node 21 (0x61b000002578): float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
Node 22 (0x61b000002590): (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 23 (0x61b0000025a8): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)))
Node 24 (0x61b0000025c0): float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0)));
Node 25 (0x61b0000025d8): 1.0
Node 26 (0x61b0000025f0): float3x3(1.0)
Node 27 (0x61b000002608): 2.0
Node 28 (0x61b000002620): float3(2.0)
Node 29 (0x61b000002638): (float3x3(1.0) * float3(2.0))
Node 30 (0x61b000002650): float3 v1 = (float3x3(1.0) * float3(2.0))
Node 31 (0x61b000002668): float3 v1 = (float3x3(1.0) * float3(2.0));
Node 32 (0x61b000002680): 2.0
Node 33 (0x61b000002698): float3(2.0)
Node 34 (0x61b0000026b0): 1.0
Node 35 (0x61b0000026c8): float3x3(1.0)
Node 36 (0x61b0000026e0): (float3(2.0) * float3x3(1.0))
Node 37 (0x61b0000026f8): float3 v2 = (float3(2.0) * float3x3(1.0))
Node 38 (0x61b000002710): float3 v2 = (float3(2.0) * float3x3(1.0));
Node 39 (0x61b000002728): sk_FragColor
Node 40 (0x61b000002740): z
Node 41 (0x61b000002758): 0
Node 42 (0x61b000002770): z[0]
Node 43 (0x61b000002788): z[0].x
Node 44 (0x61b0000027a0): half(z[0].x)
Node 45 (0x61b0000027b8): v1
Node 46 (0x61b0000027d0): v2
Node 47 (0x61b0000027e8): (v1 + v2)
Node 48 (0x61b000002800): half3((v1 + v2))
Node 49 (0x61b000002818): half4(half(z[0].x), half3((v1 + v2)))
Node 50 (0x61b000002830): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))))
Node 51 (0x61b000002848): (sk_FragColor = half4(half(z[0].x), half3((v1 + v2))));
Exits: [1]

Block 1
-------
Before: [float3 v2 = (float3(2.0) * float3x3(1.0)), float3 v1 = (float3x3(1.0) * float3(2.0)), float3x4 z = (float2x4(1.0) * float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))), float3x2 y = float3x2(1.0, 0.0, 0.0, 1.0, float2(2.0, 2.0))
