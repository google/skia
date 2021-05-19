#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float4 sk_RTAdjust;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.xy = float2(_fragCoord.x, 2.0 * _globals.sk_RTAdjust.w - _fragCoord.y);
    return _out;
}
