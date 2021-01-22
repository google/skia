#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    bool BF;
    bool BI;
    bool BB;
    float FF;
    float FI;
    float FB;
    int IF;
    int II;
    int IB;
};









fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{true, true, true, 1.2300000190734863, 1.0, 1.0, 1, 1, 1};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = float(_globals.BF);
    _out.sk_FragColor.x = float(_globals.BI);
    _out.sk_FragColor.x = float(_globals.BB);
    _out.sk_FragColor.x = _globals.FF;
    _out.sk_FragColor.x = _globals.FI;
    _out.sk_FragColor.x = _globals.FB;
    _out.sk_FragColor.x = float(_globals.IF);
    _out.sk_FragColor.x = float(_globals.II);
    _out.sk_FragColor.x = float(_globals.IB);
    return _out;
}
