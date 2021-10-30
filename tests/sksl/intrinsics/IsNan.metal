#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 testInputs;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 valueIsNaN = 0.0h / _uniforms.testInputs.yyyy;
    half4 valueIsNumber = 1.0h / _uniforms.testInputs;
    _out.sk_FragColor = ((((((isnan(float(valueIsNaN.x)) && all(isnan(float2(valueIsNaN.xy)))) && all(isnan(float3(valueIsNaN.xyz)))) && all(isnan(float4(valueIsNaN)))) && !isnan(float(valueIsNumber.x))) && !any(isnan(float2(valueIsNumber.xy)))) && !any(isnan(float3(valueIsNumber.xyz)))) && !any(isnan(float4(valueIsNumber))) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
