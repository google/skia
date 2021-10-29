#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    texture2d<float> tex;
    sampler texSmplr;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<float> tex[[texture(0)]], sampler texSmplr[[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{tex, texSmplr};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float3 _skTemp0;
    float4 a = _globals.tex.sample(_globals.texSmplr, float2(0.0));
    float4 b = _globals.tex.sample(_globals.texSmplr, (_skTemp0 = float3(0.0), _skTemp0.xy / _skTemp0.z));
    _out.sk_FragColor = float4(a.xy, b.zw);
    return _out;
}
