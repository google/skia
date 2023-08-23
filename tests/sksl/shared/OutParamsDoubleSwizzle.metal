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
half2 swizzle_lvalue_h2hhh2h(half x, half y, thread half2& color, half z) {
    color.yx = color;
    return half2(x + y, z);
}
void func_vh4(thread half4& color) {
    half2 _skTemp0;
    half2 _skTemp1;
    half2 t = ((_skTemp1 = swizzle_lvalue_h2hhh2h(1.0h, 2.0h, (_skTemp0 = color.xz), 5.0h)), (color.xz = _skTemp0), _skTemp1);
    color.yw = t;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 _skTemp2;
    half4 result = half4(0.0h, 1.0h, 2.0h, 3.0h);
    ((func_vh4((_skTemp2 = result))), (result = _skTemp2));
    _out.sk_FragColor = all(result == half4(2.0h, 3.0h, 0.0h, 5.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
