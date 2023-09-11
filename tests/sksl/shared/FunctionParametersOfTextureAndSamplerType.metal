#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
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
    float2 c  [[user(locn1)]];
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    texture2d<half, access::read> aTexture;
    sampler2D aSampledTexture;
};
half4 helpers_helper_h4ZT(Inputs _in, sampler2D s, texture2d<half, access::read> t) {
    return sample(s, _in.c);
}
half4 helper_h4TZ(Inputs _in, texture2d<half, access::read> t, sampler2D s) {
    return helpers_helper_h4ZT(_in, s, t);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<half, access::read> aTexture [[texture(1)]], texture2d<half> aSampledTexture_Tex [[texture(2)]], sampler aSampledTexture_Smplr [[sampler(2)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{aTexture, {aSampledTexture_Tex, aSampledTexture_Smplr}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = helper_h4TZ(_in, _globals.aTexture, _globals.aSampledTexture);
    return _out;
}
