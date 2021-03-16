#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    texture2d<float> texA;
    sampler texASmplr;
    texture2d<float> texB;
    sampler texBSmplr;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<float> texA[[texture(1)]], sampler texASmplr[[sampler(1)]], texture2d<float> texB[[texture(0)]], sampler texBSmplr[[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{texA, texASmplr, texB, texBSmplr};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = _globals.texA.sample(_globals.texASmplr, float2(0.0)) * _globals.texB.sample(_globals.texBSmplr, float2(0.0));
    return _out;
}
