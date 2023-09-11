#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 result;
    result.x = (_uniforms.unknownInput, _uniforms.colorGreen.x);
    result.y = (float2(2.0), _uniforms.colorGreen.y);
    result.z = (half3(3.0h), _uniforms.colorGreen.z);
    result.w = (float2x2(4.0), _uniforms.colorGreen.w);
    _out.sk_FragColor = result;
    return _out;
}
