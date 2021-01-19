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
    Globals _skGlobals{true, true, true, 1.2300000190734863, 1.0, 1.0, 1, 1, 1};
    Outputs _skOut;
    _skOut.sk_FragColor.x = float(_skGlobals.BF);
    _skOut.sk_FragColor.x = float(_skGlobals.BI);
    _skOut.sk_FragColor.x = float(_skGlobals.BB);
    _skOut.sk_FragColor.x = _skGlobals.FF;
    _skOut.sk_FragColor.x = _skGlobals.FI;
    _skOut.sk_FragColor.x = _skGlobals.FB;
    _skOut.sk_FragColor.x = float(_skGlobals.IF);
    _skOut.sk_FragColor.x = float(_skGlobals.II);
    _skOut.sk_FragColor.x = float(_skGlobals.IB);
    return _skOut;
}
