#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
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
    bool4 inputVal = bool4(_uniforms.colorGreen);
    bool4 expected = bool4(true, false, true, false);
    _out.sk_FragColor = ((((all(not(inputVal.xy) == expected.xy) && all(not(inputVal.xyz) == expected.xyz)) && all(not(inputVal) == expected)) && all(bool2(true, false) == expected.xy)) && all(bool3(true, false, true) == expected.xyz)) && all(bool4(true, false, true, false) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
