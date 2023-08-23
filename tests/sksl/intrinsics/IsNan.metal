#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
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
    _out.sk_FragColor = ((((((isnan(valueIsNaN.x) && all(isnan(valueIsNaN.xy))) && all(isnan(valueIsNaN.xyz))) && all(isnan(valueIsNaN))) && !isnan(valueIsNumber.x)) && !any(isnan(valueIsNumber.xy))) && !any(isnan(valueIsNumber.xyz))) && !any(isnan(valueIsNumber)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
