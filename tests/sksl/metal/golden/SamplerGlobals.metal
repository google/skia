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
    Globals _skGlobals{texA, texASmplr, texB, texBSmplr};
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = _skGlobals.texA.sample(_skGlobals.texASmplr, float2(0.0)) * _skGlobals.texB.sample(_skGlobals.texBSmplr, float2(0.0));
    return *_out;
}
