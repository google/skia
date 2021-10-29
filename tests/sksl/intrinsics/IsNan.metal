#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 valueIsNaN = 0.0 / _uniforms.testInputs.yyyy;
    float4 valueIsNumber = 1.0 / _uniforms.testInputs;
    _out.sk_FragColor = ((((((isnan(valueIsNaN.x) && all(isnan(valueIsNaN.xy))) && all(isnan(valueIsNaN.xyz))) && all(isnan(valueIsNaN))) && !isnan(valueIsNumber.x)) && !any(isnan(valueIsNumber.xy))) && !any(isnan(valueIsNumber.xyz))) && !any(isnan(valueIsNumber)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
