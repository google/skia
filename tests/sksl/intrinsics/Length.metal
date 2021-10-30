#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 inputVal;
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
    half4 expected = half4(3.0h, 3.0h, 5.0h, 13.0h);
    _out.sk_FragColor = ((((((abs(_uniforms.inputVal.x) == expected.x && length(_uniforms.inputVal.xy) == expected.y) && length(_uniforms.inputVal.xyz) == expected.z) && length(_uniforms.inputVal) == expected.w) && 3.0h == expected.x) && 3.0h == expected.y) && 5.0h == expected.z) && 13.0h == expected.w ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
