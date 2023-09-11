#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float4 testInputs;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    uint xy = pack_float_to_unorm2x16(_uniforms.testInputs.xy);
    uint zw = pack_float_to_unorm2x16(_uniforms.testInputs.zw);
    const float2 tolerance = float2(0.015625);
    _out.sk_FragColor = all((abs(unpack_unorm2x16_to_float(xy)) < tolerance)) && all((abs(unpack_unorm2x16_to_float(zw) - float2(0.75, 1.0)) < tolerance)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
