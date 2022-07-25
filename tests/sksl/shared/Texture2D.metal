#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

struct sampler2D {
    texture2d<half> tex;
    sampler smp;
};
half4 sample(sampler2D i, float2 p, float b=0) { return i.tex.sample(i.smp, p, bias(b)); }
half4 sample(sampler2D i, float3 p, float b=0) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }

struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    sampler2D tex;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<half> tex_Tex [[texture(0)]], sampler tex_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{tex_Tex, tex_Smplr}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float4 a = float4(sample(_globals.tex, float2(0.0)));
    float4 b = float4(sample(_globals.tex, float3(0.0)));
    _out.sk_FragColor = half4(half2(a.xy), half2(b.zw));
    return _out;
}
