#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4x4 colorXform;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    texture2d<float> s;
    sampler sSmplr;
};


fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], texture2d<float> s[[texture(0)]], sampler sSmplr[[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{s, sSmplr};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 tmpColor;
    _out->sk_FragColor = (tmpColor = _globals->s.sample(_globals->sSmplr, float2(1.0)) , _uniforms.colorXform != float4x4(1.0) ? float4(clamp((_uniforms.colorXform * float4(tmpColor.xyz, 1.0)).xyz, 0.0, tmpColor.w), tmpColor.w) : tmpColor);
    return *_out;
}
