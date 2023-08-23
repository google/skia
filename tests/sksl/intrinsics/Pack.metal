#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half2 a;
    half4 b;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = half(as_type<uint>(half2(float2(_uniforms.a))));
    _out.sk_FragColor.x = half(pack_float_to_unorm2x16(float2(_uniforms.a)));
    _out.sk_FragColor.x = half(pack_float_to_snorm2x16(float2(_uniforms.a)));
    _out.sk_FragColor.x = half(pack_float_to_unorm4x8(float4(_uniforms.b)));
    _out.sk_FragColor.x = half(pack_float_to_snorm4x8(float4(_uniforms.b)));
    return _out;
}
