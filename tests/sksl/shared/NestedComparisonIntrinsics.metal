#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorRed;
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool4 result = ((_uniforms.colorRed < half4(2.0h)) == (half4(3.0h) > _uniforms.colorGreen));
    _out.sk_FragColor = all(result) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
