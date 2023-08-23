#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half a;
    half b;
    half c;
    half4 d;
    half4 e;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 result = half4((refract(float2(6e+26h, 0), float2(2.0h, 0), 2.0h).x));
    result.x = (refract(float2(_uniforms.a, 0), float2(_uniforms.b, 0), _uniforms.c).x);
    result = refract(_uniforms.d, _uniforms.e, _uniforms.c);
    result.xy = half2(0.5h, -0.8660254h);
    result.xyz = half3(0.5h, 0.0h, -0.8660254h);
    result = half4(0.5h, 0.0h, 0.0h, -0.8660254h);
    _out.sk_FragColor = result;
    return _out;
}
