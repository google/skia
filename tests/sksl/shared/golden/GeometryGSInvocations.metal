### Compilation failed:

Block 0
-------
Before: []
Entrances: []
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): -0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
Node 17 (0x617000002898): EndPrimitive()
Node 18 (0x6170000028b0): EndPrimitive();
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
