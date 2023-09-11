#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float3x3 testMatrix3x3;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);
float2x2 float2x2_from_float3x3(float3x3 x0) {
    return float2x2(float2(x0[0].xy), float2(x0[1].xy));
}
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
float3x3 float3x3_from_float2x2(float2x2 x0) {
    return float3x3(float3(x0[0].xy, 0.0), float3(x0[1].xy, 0.0), float3(0.0, 0.0, 1.0));
}
float2x2 resizeMatrix_f22(Uniforms _uniforms) {
    return float2x2_from_float3x3(_uniforms.testMatrix3x3);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = resizeMatrix_f22(_uniforms) == float2x2(float2(1.0, 2.0), float2(4.0, 5.0)) && float3x3_from_float2x2(resizeMatrix_f22(_uniforms)) == float3x3(float3(1.0, 2.0, 0.0), float3(4.0, 5.0, 0.0), float3(0.0, 0.0, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
