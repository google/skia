#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 inputVal;
    half4 expected;
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
    _out.sk_FragColor = ((((((asin(_uniforms.inputVal.x) == _uniforms.expected.x && all(asin(_uniforms.inputVal.xy) == _uniforms.expected.xy)) && all(asin(_uniforms.inputVal.xyz) == _uniforms.expected.xyz)) && all(asin(_uniforms.inputVal) == _uniforms.expected)) && 0.0h == _uniforms.expected.x) && all(half2(0.0h) == _uniforms.expected.xy)) && all(half3(0.0h) == _uniforms.expected.xyz)) && all(half4(0.0h) == _uniforms.expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
