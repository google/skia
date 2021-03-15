#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float2 h2;
    float3 h3;
    float4 h4;
    float2 f2;
    float3 f3;
    float4 f4;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}, {}, {}, {}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = outerProduct(_globals.f2, _globals.f2)[1].xyyy;
    _out.sk_FragColor = outerProduct(_globals.f3, _globals.f3)[2].xyzz;
    _out.sk_FragColor = outerProduct(_globals.f4, _globals.f4)[3];
    _out.sk_FragColor = outerProduct(_globals.f3, _globals.f2)[1].xyzz;
    _out.sk_FragColor = outerProduct(_globals.f2, _globals.f3)[2].xyyy;
    _out.sk_FragColor = outerProduct(_globals.f4, _globals.f2)[1];
    _out.sk_FragColor = outerProduct(_globals.f2, _globals.f4)[3].xyyy;
    _out.sk_FragColor = outerProduct(_globals.f4, _globals.f3)[2];
    _out.sk_FragColor = outerProduct(_globals.f3, _globals.f4)[3].xyzz;
    _out.sk_FragColor = outerProduct(_globals.h2, _globals.h2)[1].xyyy;
    _out.sk_FragColor = outerProduct(_globals.h3, _globals.h3)[2].xyzz;
    _out.sk_FragColor = outerProduct(_globals.h4, _globals.h4)[3];
    _out.sk_FragColor = outerProduct(_globals.h3, _globals.h2)[1].xyzz;
    _out.sk_FragColor = outerProduct(_globals.h2, _globals.h3)[2].xyyy;
    _out.sk_FragColor = outerProduct(_globals.h4, _globals.h2)[1];
    _out.sk_FragColor = outerProduct(_globals.h2, _globals.h4)[3].xyyy;
    _out.sk_FragColor = outerProduct(_globals.h4, _globals.h3)[2];
    _out.sk_FragColor = outerProduct(_globals.h3, _globals.h4)[3].xyzz;
    return _out;
}
