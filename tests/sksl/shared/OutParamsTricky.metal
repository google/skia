#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float2 tricky(float x, float y, thread float2& color, float z);
float2 _skOutParamHelper0_tricky(float _var0, float _var1, thread float4& color, float _var3) {
    float2 _var2 = color.xz;
    float2 _skResult = tricky(_var0, _var1, _var2, _var3);
    color.xz = _var2;
    return _skResult;
}
void func(thread float4& color);
void _skOutParamHelper1_func(thread float4& result) {
    float4 _var0 = result;
    func(_var0);
    result = _var0;
}

float2 tricky(float x, float y, thread float2& color, float z) {
    color = color.yx;
    return float2(x + y, z);
}
void func(thread float4& color) {
    float2 t =     _skOutParamHelper0_tricky(1.0, 2.0, color, 5.0);
    color.yw = t;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 result = float4(0.0, 1.0, 2.0, 3.0);
    _skOutParamHelper1_func(result);
    _out.sk_FragColor = all(result == float4(2.0, 3.0, 0.0, 5.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
