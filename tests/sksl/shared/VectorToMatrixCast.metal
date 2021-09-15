#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float4 testInputs;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0.xy), float2(x0.zw));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool ok = true;
    ok = ok && float2x2_from_float4(_uniforms.testInputs) == float2x2(float2(-1.25, 0.0), float2(0.75, 2.25));
    ok = ok && float2x2_from_float4(_uniforms.testInputs) == float2x2(float2(-1.25, 0.0), float2(0.75, 2.25));
    ok = ok && float2x2_from_float4(_uniforms.colorGreen) == float2x2(float2(0.0, 1.0), float2(0.0, 1.0));
    ok = ok && float2x2_from_float4(_uniforms.colorGreen) == float2x2(float2(0.0, 1.0), float2(0.0, 1.0));
    ok = ok && float2x2_from_float4(float4(int4(_uniforms.colorGreen))) == float2x2(float2(0.0, 1.0), float2(0.0, 1.0));
    ok = ok && float2x2_from_float4(_uniforms.colorGreen) == float2x2(float2(0.0, 1.0), float2(0.0, 1.0));
    ok = ok && float2x2_from_float4(_uniforms.colorGreen) == float2x2(float2(0.0, 1.0), float2(0.0, 1.0));
    ok = ok && float2x2_from_float4(float4(bool4(_uniforms.colorGreen))) == float2x2(float2(0.0, 1.0), float2(0.0, 1.0));
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
