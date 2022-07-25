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
    sampler2D texA;
    sampler2D texB;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<half> texA_Tex [[texture(1)]], sampler texA_Smplr [[sampler(1)]], texture2d<half> texB_Tex [[texture(0)]], sampler texB_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{texA_Tex, texA_Smplr}, {texB_Tex, texB_Smplr}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = sample(_globals.texA, float2(0.0)) * sample(_globals.texB, float2(0.0));
    return _out;
}
