#include <metal_stdlib>
#include <simd/simd.h>
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
half2 tricky_h2hhh2h(half x, half y, thread half2& color, half z);
half2 _skOutParamHelper0_tricky_h2hhh2h(half _var0, half _var1, thread half4& color, half _var3) {
    half2 _var2 = color.xz;
    half2 _skResult = tricky_h2hhh2h(_var0, _var1, _var2, _var3);
    color.xz = _var2;
    return _skResult;
}
void func_vh4(thread half4& color);
void _skOutParamHelper1_func_vh4(thread half4& result) {
    half4 _var0 = result;
    func_vh4(_var0);
    result = _var0;
}
half2 tricky_h2hhh2h(half x, half y, thread half2& color, half z) {
    color = color.yx;
    return half2(x + y, z);
}
void func_vh4(thread half4& color) {
    half2 t =     _skOutParamHelper0_tricky_h2hhh2h(1.0h, 2.0h, color, 5.0h);
    color.yw = t;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 result = half4(0.0h, 1.0h, 2.0h, 3.0h);
    _skOutParamHelper1_func_vh4(result);
    _out.sk_FragColor = all(result == half4(2.0h, 3.0h, 0.0h, 5.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
