#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorRed;
    half4 colorGreen;
    half4 testInputs;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half fn_hh4(half4 v) {
    for (int x = 1;x <= 2; ++x) {
        return v.x;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 v = _uniforms.testInputs;
    v = half4(0.0h, v.zyx);
    v = half4(0.0h, 0.0h, v.xw);
    v = half4(1.0h, 1.0h, v.wx);
    v = half4(v.zy, 1.0h, 1.0h);
    v = half4(v.xx, 1.0h, 1.0h);
    v = v.wzwz;
    v = half3(fn_hh4(v), 123.0h, 456.0h).yyzz;
    v = half3(fn_hh4(v), 123.0h, 456.0h).yyzz;
    v = half4(123.0h, 456.0h, 456.0h, fn_hh4(v));
    v = half4(123.0h, 456.0h, 456.0h, fn_hh4(v));
    v = half3(fn_hh4(v), 123.0h, 456.0h).yxxz;
    v = half3(fn_hh4(v), 123.0h, 456.0h).yxxz;
    v = half4(1.0h, 1.0h, 2.0h, 3.0h);
    v = half4(_uniforms.colorRed.xyz, 1.0h);
    v = half4(_uniforms.colorRed.x, 1.0h, _uniforms.colorRed.yz);
    v.wzyx = v;
    v.xw = v.yz;
    v.zyx = half3(v.ww, 1.0h);
    _out.sk_FragColor = all(v == half4(1.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
