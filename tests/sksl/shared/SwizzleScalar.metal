#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half unknownInput;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 h4 = half4(_uniforms.unknownInput);
    h4 = half4(half2(_uniforms.unknownInput), 0.0h, 1.0h);
    h4 = half4(0.0h, _uniforms.unknownInput, 1.0h, 0.0h);
    h4 = half4(0.0h, _uniforms.unknownInput, 0.0h, _uniforms.unknownInput);
    _out.sk_FragColor = h4;
    return _out;
}
