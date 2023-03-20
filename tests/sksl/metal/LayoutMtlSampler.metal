#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

struct sampler2D {
    texture2d<half> tex;
    sampler smp;
};
half4 sample(sampler2D i, float2 p, float b=0) { return i.tex.sample(i.smp, p, bias(b)); }
half4 sample(sampler2D i, float3 p, float b=0) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }
half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }
half4 sampleLod(sampler2D i, float3 p, float lod) {
    return i.tex.sample(i.smp, p.xy / p.z, level(lod));
}
half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {
    return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));
}

struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    sampler2D mtlSampler;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<half> mtlSampler_Tex [[texture(0)]], sampler mtlSampler_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{mtlSampler_Tex, mtlSampler_Smplr}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = sample(_globals.mtlSampler, float2(0.0));
    return _out;
}
