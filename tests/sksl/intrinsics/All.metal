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
    bool4 inputVal = bool4(_uniforms.colorRed.xxzw);
    bool4 expected = bool4(_uniforms.colorRed.xyzz);
    _out.sk_FragColor = ((((all(inputVal.xy) == expected.x && all(inputVal.xyz) == expected.y) && all(inputVal) == expected.z) && expected.x) && false == expected.y) && false == expected.z ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
