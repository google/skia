#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
void outParameterWrite_vh4(Uniforms _uniforms, thread half4& x) {
    x = _uniforms.colorGreen;
}
void outParameterWriteIndirect_vh4(Uniforms _uniforms, thread half4& c) {
    half4 _skTemp0;
    ((outParameterWrite_vh4(_uniforms, _skTemp0)), (c = _skTemp0));
}
void inoutParameterWrite_vh4(thread half4& x) {
    x *= x;
}
void inoutParameterWriteIndirect_vh4(thread half4& x) {
    half4 _skTemp1;
    ((inoutParameterWrite_vh4((_skTemp1 = x))), (x = _skTemp1));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 _skTemp2;
    half4 _skTemp3;
    half4 _skTemp4;
    half4 _skTemp5;
    half4 c;
    ((outParameterWrite_vh4(_uniforms, _skTemp2)), (c = _skTemp2));
    ((outParameterWriteIndirect_vh4(_uniforms, _skTemp3)), (c = _skTemp3));
    ((inoutParameterWrite_vh4((_skTemp4 = c))), (c = _skTemp4));
    ((inoutParameterWriteIndirect_vh4((_skTemp5 = c))), (c = _skTemp5));
    _out.sk_FragColor = c;
    return _out;
}
