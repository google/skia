### Compilation failed:

Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex.sk_Position 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_in 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_in[0] 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_in[0].sk_Position 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX -0.5 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_InvocationID 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX float(sk_InvocationID) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX float4(-0.5, 0.0, 0.0, float(sk_InvocationID)) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX EmitVertex() 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST EmitVertex(); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex.sk_Position 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_in 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_in[0] 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_in[0].sk_Position 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX 0.5 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_InvocationID 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX float(sk_InvocationID) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX float4(0.5, 0.0, 0.0, float(sk_InvocationID)) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX EmitVertex() 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST EmitVertex(); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX EndPrimitive() 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002380): sk_PerVertex
Node 1 (0x61b000002398): sk_PerVertex.sk_Position
Node 2 (0x61b0000023b0): sk_in
Node 3 (0x61b0000023c8): 0
Node 4 (0x61b0000023e0): sk_in[0]
Node 5 (0x61b0000023f8): sk_in[0].sk_Position
Node 6 (0x61b000002410): -0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): sk_PerVertex
Node 18 (0x61b000002530): sk_PerVertex.sk_Position
Node 19 (0x61b000002548): sk_in
Node 20 (0x61b000002560): 0
Node 21 (0x61b000002578): sk_in[0]
Node 22 (0x61b000002590): sk_in[0].sk_Position
Node 23 (0x61b0000025a8): 0.5
Node 24 (0x61b0000025c0): 0.0
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): sk_InvocationID
Node 27 (0x61b000002608): float(sk_InvocationID)
Node 28 (0x61b000002620): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002638): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002650): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002680): EmitVertex()
Node 33 (0x61b000002698): EmitVertex();
Node 34 (0x61b0000026b0): EndPrimitive()
Node 35 (0x61b0000026c8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST EndPrimitive(); 
simplifying expr
error: 1: unsupported kind of program
1 error
