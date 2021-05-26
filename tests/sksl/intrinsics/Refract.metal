#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float a;
    float b;
    float c;
    float4 d;
    float4 e;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = (refract(float2(_uniforms.a, 0), float2(_uniforms.b, 0), _uniforms.c).x);
    _out.sk_FragColor = refract(_uniforms.d, _uniforms.e, _uniforms.c);
    _out.sk_FragColor.xy = float2(0.5, -0.86602538824081421);
    _out.sk_FragColor.xyz = float3(0.5, 0.0, -0.86602538824081421);
    _out.sk_FragColor = float4(0.5, 0.0, 0.0, -0.86602538824081421);
    return _out;
}
