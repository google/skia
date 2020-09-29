### Compilation failed:

Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex.sk_Position.xy 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_RTAdjust 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_RTAdjust.xz 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex.sk_Position.ww 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_RTAdjust 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_RTAdjust.yw 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex.sk_Position.w 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex.sk_Position.xy 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_RTAdjust 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_RTAdjust.xz 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex.sk_Position.ww 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_RTAdjust 
optimizing varref 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_RTAdjust.yw 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX sk_PerVertex.sk_Position.w 
optimizing swiz 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify EX (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)); 
simplifying expr
Block 0
-------
Before: []
Entrances: []
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
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
Node 0 (0x61f000000e80): sk_PerVertex
Node 1 (0x61f000000e98): sk_PerVertex.sk_Position
Node 2 (0x61f000000eb0): sk_in
Node 3 (0x61f000000ec8): 0
Node 4 (0x61f000000ee0): sk_in[0]
Node 5 (0x61f000000ef8): sk_in[0].sk_Position
Node 6 (0x61f000000f10): -0.5
Node 7 (0x61f000000f28): 0.0
Node 8 (0x61f000000f40): 0.0
Node 9 (0x61f000000f58): sk_InvocationID
Node 10 (0x61f000000f70): float(sk_InvocationID)
Node 11 (0x61f000000f88): float4(-0.5, 0.0, 0.0, float(sk_InvocationID))
Node 12 (0x61f000000fa0): (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 13 (0x61f000000fb8): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 14 (0x61f000000fd0): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(-0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 15 (0x61f000000fe8): sk_PerVertex
Node 16 (0x61f000001000): sk_PerVertex.sk_Position
Node 17 (0x61f000001018): sk_PerVertex
Node 18 (0x61f000001030): sk_PerVertex.sk_Position
Node 19 (0x61f000001048): sk_PerVertex.sk_Position.xy
Node 20 (0x61f000001060): sk_RTAdjust
Node 21 (0x61f000001078): sk_RTAdjust.xz
Node 22 (0x61f000001090): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 23 (0x61f0000010a8): sk_PerVertex
Node 24 (0x61f0000010c0): sk_PerVertex.sk_Position
Node 25 (0x61f0000010d8): sk_PerVertex.sk_Position.ww
Node 26 (0x61f0000010f0): sk_RTAdjust
Node 27 (0x61f000001108): sk_RTAdjust.yw
Node 28 (0x61f000001120): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 29 (0x61f000001138): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 30 (0x61f000001150): 0.0
Node 31 (0x61f000001168): sk_PerVertex
Node 32 (0x61f000001180): sk_PerVertex.sk_Position
Node 33 (0x61f000001198): sk_PerVertex.sk_Position.w
Node 34 (0x61f0000011b0): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 35 (0x61f0000011c8): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 36 (0x61f0000011e0): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 37 (0x61f0000011f8): EmitVertex()
Node 38 (0x61f000001210): EmitVertex();
Node 39 (0x61f000001228): sk_PerVertex
Node 40 (0x61f000001240): sk_PerVertex.sk_Position
Node 41 (0x61f000001258): sk_in
Node 42 (0x61f000001270): 0
Node 43 (0x61f000001288): sk_in[0]
Node 44 (0x61f0000012a0): sk_in[0].sk_Position
Node 45 (0x61f0000012b8): 0.5
Node 46 (0x61f0000012d0): 0.0
Node 47 (0x61f0000012e8): 0.0
Node 48 (0x61f000001300): sk_InvocationID
Node 49 (0x61f000001318): float(sk_InvocationID)
Node 50 (0x61f000001330): float4(0.5, 0.0, 0.0, float(sk_InvocationID))
Node 51 (0x61f000001348): (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID)))
Node 52 (0x61f000001360): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))))
Node 53 (0x61f000001378): (sk_PerVertex.sk_Position = (sk_in[0].sk_Position + float4(0.5, 0.0, 0.0, float(sk_InvocationID))));
Node 54 (0x61f000001390): sk_PerVertex
Node 55 (0x61f0000013a8): sk_PerVertex.sk_Position
Node 56 (0x61f0000013c0): sk_PerVertex
Node 57 (0x61f0000013d8): sk_PerVertex.sk_Position
Node 58 (0x61f0000013f0): sk_PerVertex.sk_Position.xy
Node 59 (0x61f000001408): sk_RTAdjust
Node 60 (0x61f000001420): sk_RTAdjust.xz
Node 61 (0x61f000001438): (sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz)
Node 62 (0x61f000001450): sk_PerVertex
Node 63 (0x61f000001468): sk_PerVertex.sk_Position
Node 64 (0x61f000001480): sk_PerVertex.sk_Position.ww
Node 65 (0x61f000001498): sk_RTAdjust
Node 66 (0x61f0000014b0): sk_RTAdjust.yw
Node 67 (0x61f0000014c8): (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)
Node 68 (0x61f0000014e0): ((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw))
Node 69 (0x61f0000014f8): 0.0
Node 70 (0x61f000001510): sk_PerVertex
Node 71 (0x61f000001528): sk_PerVertex.sk_Position
Node 72 (0x61f000001540): sk_PerVertex.sk_Position.w
Node 73 (0x61f000001558): float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w)
Node 74 (0x61f000001570): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w))
Node 75 (0x61f000001588): (sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + (sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));
Node 76 (0x61f0000015a0): EmitVertex()
Node 77 (0x61f0000015b8): EmitVertex();
Node 78 (0x61f0000015d0): EndPrimitive()
Node 79 (0x61f0000015e8): EndPrimitive();
Exits: [1]

Block 1
-------
Before: []
Entrances: [0]
Exits: []

about to simplify ST EndPrimitive(); 
simplifying expr
error: 1: Metal uniforms must have 'layout(set=...)'
error: 1: unsupported kind of program
2 errors
