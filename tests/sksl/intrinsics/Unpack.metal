#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    uint a;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.xy = half2(float2(as_type<half2>(_uniforms.a)));
    _out.sk_FragColor.xy = half2(unpack_unorm2x16_to_float(_uniforms.a));
    _out.sk_FragColor.xy = half2(unpack_snorm2x16_to_float(_uniforms.a));
    _out.sk_FragColor = half4(unpack_unorm4x8_to_float(_uniforms.a));
    _out.sk_FragColor = half4(unpack_snorm4x8_to_float(_uniforms.a));
    return _out;
}
