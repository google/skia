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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 0 (0x617000002700): sk_PerVertex
Node 1 (0x617000002718): sk_PerVertex.sk_Position
Node 2 (0x617000002730): sk_in
Node 3 (0x617000002748): 0
Node 4 (0x617000002760): sk_in[0]
Node 5 (0x617000002778): sk_in[0].sk_Position
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x617000002790): 0.5
Node 7 (0x6170000027a8): 0.0
Node 8 (0x6170000027c0): 0.0
Node 9 (0x6170000027d8): sk_InvocationID
Node 10 (0x6170000027f0): float(sk_InvocationID)
Node 11 (0x617000002808): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x617000002820): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x617000002838): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x617000002850): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x617000002868): EmitVertex()
Node 16 (0x617000002880): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX false 
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): false
Node 18 (0x61b000002530): false;
Node 19 (0x61b000002548): sk_PerVertex
Node 20 (0x61b000002560): sk_PerVertex.sk_Position
Node 21 (0x61b000002578): sk_in
Node 22 (0x61b000002590): 0
Node 23 (0x61b0000025a8): sk_in[0]
Node 24 (0x61b0000025c0): sk_in[0].sk_Position
Node 25 (0x61b0000025d8): -0.5
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): 0.0
Node 28 (0x61b000002620): sk_InvocationID
Node 29 (0x61b000002638): float(sk_InvocationID)
Node 30 (0x61b000002650): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 31 (0x61b000002668): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 33 (0x61b000002698): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 34 (0x61b0000026b0): EmitVertex()
Node 35 (0x61b0000026c8): EmitVertex();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST false; 
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))) 
optimizing binary 
deadass? update=1 rescan=0 
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))) 
optimizing binary 
deadass? update=1 rescan=0 
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST ; 
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 6 (0x61b000002410): 0.5
Node 7 (0x61b000002428): 0.0
Node 8 (0x61b000002440): 0.0
Node 9 (0x61b000002458): sk_InvocationID
Node 10 (0x61b000002470): float(sk_InvocationID)
Node 11 (0x61b000002488): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b0000024a0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b0000024b8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b0000024d0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b0000024e8): EmitVertex()
Node 16 (0x61b000002500): EmitVertex();
Node 17 (0x61b000002518): ;
Node 18 (0x61b000002530): sk_PerVertex
Node 19 (0x61b000002548): sk_PerVertex.sk_Position
Node 20 (0x61b000002560): sk_in
Node 21 (0x61b000002578): 0
Node 22 (0x61b000002590): sk_in[0]
Node 23 (0x61b0000025a8): sk_in[0].sk_Position
Node 24 (0x61b0000025c0): -0.5
Node 25 (0x61b0000025d8): 0.0
Node 26 (0x61b0000025f0): 0.0
Node 27 (0x61b000002608): sk_InvocationID
Node 28 (0x61b000002620): float(sk_InvocationID)
Node 29 (0x61b000002638): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 30 (0x61b000002650): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 31 (0x61b000002668): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 32 (0x61b000002680): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 33 (0x61b000002698): EmitVertex()
Node 34 (0x61b0000026b0): EmitVertex();
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
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX sk_InvocationID 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX (sk_InvocationID = 0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify ST (sk_InvocationID = 0); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX sk_InvocationID 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX 2 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX (sk_InvocationID < 2) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX sk_InvocationID 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX sk_InvocationID++ 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX _invoke() 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify ST _invoke(); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX EndPrimitive() 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x608000010ea0): sk_InvocationID
Node 1 (0x608000010eb8): 0
Node 2 (0x608000010ed0): (sk_InvocationID = 0)
Node 3 (0x608000010ee8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000010f20): sk_InvocationID
Node 1 (0x608000010f38): 2
Node 2 (0x608000010f50): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x60400002b890): sk_InvocationID
Node 1 (0x60400002b8a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x608000010fa0): _invoke()
Node 1 (0x608000010fb8): _invoke();
Node 2 (0x608000010fd0): EndPrimitive()
Node 3 (0x608000010fe8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify ST EndPrimitive(); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x61b000002a80): sk_PerVertex
Node 1 (0x61b000002a98): sk_PerVertex.sk_Position
Node 2 (0x61b000002ab0): sk_in
Node 3 (0x61b000002ac8): 0
Node 4 (0x61b000002ae0): sk_in[0]
Node 5 (0x61b000002af8): sk_in[0].sk_Position
Node 6 (0x61b000002b10): 0.5
Node 7 (0x61b000002b28): 0.0
Node 8 (0x61b000002b40): 0.0
Node 9 (0x61b000002b58): sk_InvocationID
Node 10 (0x61b000002b70): float(sk_InvocationID)
Node 11 (0x61b000002b88): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61b000002ba0): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61b000002bb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61b000002bd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61b000002be8): EmitVertex()
Node 16 (0x61b000002c00): EmitVertex();
Node 17 (0x61b000002c18): sk_PerVertex
Node 18 (0x61b000002c30): sk_PerVertex.sk_Position
Node 19 (0x61b000002c48): sk_in
Node 20 (0x61b000002c60): 0
Node 21 (0x61b000002c78): sk_in[0]
Node 22 (0x61b000002c90): sk_in[0].sk_Position
Node 23 (0x61b000002ca8): -0.5
Node 24 (0x61b000002cc0): 0.0
Node 25 (0x61b000002cd8): 0.0
Node 26 (0x61b000002cf0): sk_InvocationID
Node 27 (0x61b000002d08): float(sk_InvocationID)
Node 28 (0x61b000002d20): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 29 (0x61b000002d38): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 30 (0x61b000002d50): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 31 (0x61b000002d68): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 32 (0x61b000002d80): EmitVertex()
Node 33 (0x61b000002d98): EmitVertex();
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
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX sk_InvocationID 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX (sk_InvocationID = 0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify ST (sk_InvocationID = 0); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX sk_InvocationID 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX 2 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX (sk_InvocationID < 2) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX sk_InvocationID 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX sk_InvocationID++ 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX _invoke() 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify ST _invoke(); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify EX EndPrimitive() 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000110a0): sk_InvocationID
Node 1 (0x6080000110b8): 0
Node 2 (0x6080000110d0): (sk_InvocationID = 0)
Node 3 (0x6080000110e8): (sk_InvocationID = 0);
Exits: [1]

Block 1
-------
Before: []
Entrances: [0, 2]
Node 0 (0x608000011120): sk_InvocationID
Node 1 (0x608000011138): 2
Node 2 (0x608000011150): (sk_InvocationID < 2)
Exits: [4]

Block 2
-------
Before: []
Entrances: [4]
Node 0 (0x604000039490): sk_InvocationID
Node 1 (0x6040000394a8): sk_InvocationID++
Exits: [1, 3]

Block 3
-------
Before: []
Entrances: [2]
Exits: [5]

Block 4
-------
Before: []
Entrances: [1]
Node 0 (0x6080000111a0): _invoke()
Node 1 (0x6080000111b8): _invoke();
Node 2 (0x6080000111d0): EndPrimitive()
Node 3 (0x6080000111e8): EndPrimitive();
Exits: [2]

Block 5
-------
Before: []
Entrances: [3]
Exits: []

about to simplify ST EndPrimitive(); 
simplifying expr
error: 1: unsupported kind of program
1 error
