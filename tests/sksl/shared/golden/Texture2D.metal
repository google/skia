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
    Globals _skGlobals{tex, texSmplr};
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float3 _skTemp0;
    float4 a = _skGlobals.tex.sample(_skGlobals.texSmplr, float2(0.0));
    float4 b = _skGlobals.tex.sample(_skGlobals.texSmplr, (_skTemp0 = float3(0.0), _skTemp0.xy / _skTemp0.z));
    _out->sk_FragColor = float4(a.xy, b.zw);
    return *_out;
}
