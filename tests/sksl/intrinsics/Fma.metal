#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    array<float, 5> testArray;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float one = _uniforms.testArray[0];
    float two = _uniforms.testArray[1];
    float three = _uniforms.testArray[2];
    half four = half(_uniforms.testArray[3]);
    half five = half(_uniforms.testArray[4]);
    _out.sk_FragColor = fma(one, two, three) == 5.0 && fma(half(three), four, five) == 17.0h ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
