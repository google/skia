#include <metal_stdlib>
#include <simd/simd.h>
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
    _out.sk_FragColor = ((((((((_uniforms.inputVal.x) * 57.2957795) == _uniforms.expected.x && all(((_uniforms.inputVal.xy) * 57.2957795) == _uniforms.expected.xy)) && all(((_uniforms.inputVal.xyz) * 57.2957795) == _uniforms.expected.xyz)) && all(((_uniforms.inputVal) * 57.2957795) == _uniforms.expected)) && 90.0h == _uniforms.expected.x) && all(half2(90.0h, 180.0h) == _uniforms.expected.xy)) && all(half3(90.0h, 180.0h, 270.0h) == _uniforms.expected.xyz)) && all(half4(90.0h, 180.0h, 270.0h, 360.0h) == _uniforms.expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
