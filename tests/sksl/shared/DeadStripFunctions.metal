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
half4 unpremul_h4h4(half4 color);
half4 unpremul_h4h4(half4 color) {
    return half4(color.xyz / max(color.w, 0.0001h), color.w);
}
half4 live_fn_h4h4h4(half4 a, half4 b) {
    return a + b;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 a;
    half4 b;
    {
        a = live_fn_h4h4h4(half4(3.0h), half4(-5.0h));
    }
    {
        b = unpremul_h4h4(half4(1.0h));
    }
    _out.sk_FragColor = any(a != half4(0.0h)) && any(b != half4(0.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
