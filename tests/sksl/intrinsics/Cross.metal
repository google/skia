#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half2 ah;
    half2 bh;
    float2 af;
    float2 bf;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = determinant(half2x2(_uniforms.ah, _uniforms.bh));
    _out.sk_FragColor.y = half(determinant(float2x2(_uniforms.af, _uniforms.bf)));
    _out.sk_FragColor.z = 12.0h;
    _out.sk_FragColor.xyz = half3(-8.0h, -8.0h, 12.0h);
    _out.sk_FragColor.yzw = half3(9.0h, -18.0h, -9.0h);
    return _out;
}
