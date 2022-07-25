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
    sampler2D test2D;
    sampler2D test2DRect;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<half> test2D_Tex [[texture(0)]], sampler test2D_Smplr [[sampler(0)]], texture2d<half> test2DRect_Tex [[texture(1)]], sampler test2DRect_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{test2D_Tex, test2D_Smplr}, {test2DRect_Tex, test2DRect_Smplr}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = sample(_globals.test2D, float2(0.5));
    _out.sk_FragColor = sample(_globals.test2DRect, float2(0.5));
    _out.sk_FragColor = sample(_globals.test2DRect, float3(0.5));
    return _out;
}
