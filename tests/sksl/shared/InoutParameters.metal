#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
void outParameterWrite_vh4(Uniforms _uniforms, thread half4& x);
void _skOutParamHelper0_outParameterWrite_vh4(Uniforms _uniforms, thread half4& c) {
    half4 _var0;
    outParameterWrite_vh4(_uniforms, _var0);
    c = _var0;
}
void inoutParameterWrite_vh4(thread half4& x);
void _skOutParamHelper1_inoutParameterWrite_vh4(thread half4& x) {
    half4 _var0 = x;
    inoutParameterWrite_vh4(_var0);
    x = _var0;
}
void outParameterWrite_vh4(Uniforms _uniforms, thread half4& x);
void _skOutParamHelper2_outParameterWrite_vh4(Uniforms _uniforms, thread half4& c) {
    half4 _var0;
    outParameterWrite_vh4(_uniforms, _var0);
    c = _var0;
}
void outParameterWriteIndirect_vh4(Uniforms _uniforms, thread half4& c);
void _skOutParamHelper3_outParameterWriteIndirect_vh4(Uniforms _uniforms, thread half4& c) {
    half4 _var0;
    outParameterWriteIndirect_vh4(_uniforms, _var0);
    c = _var0;
}
void inoutParameterWrite_vh4(thread half4& x);
void _skOutParamHelper4_inoutParameterWrite_vh4(thread half4& c) {
    half4 _var0 = c;
    inoutParameterWrite_vh4(_var0);
    c = _var0;
}
void inoutParameterWriteIndirect_vh4(thread half4& x);
void _skOutParamHelper5_inoutParameterWriteIndirect_vh4(thread half4& c) {
    half4 _var0 = c;
    inoutParameterWriteIndirect_vh4(_var0);
    c = _var0;
}
void outParameterWrite_vh4(Uniforms _uniforms, thread half4& x) {
    x = _uniforms.colorGreen;
}
void outParameterWriteIndirect_vh4(Uniforms _uniforms, thread half4& c) {
    _skOutParamHelper0_outParameterWrite_vh4(_uniforms, c);
}
void inoutParameterWrite_vh4(thread half4& x) {
    x *= x;
}
void inoutParameterWriteIndirect_vh4(thread half4& x) {
    _skOutParamHelper1_inoutParameterWrite_vh4(x);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 c;
    _skOutParamHelper2_outParameterWrite_vh4(_uniforms, c);
    _skOutParamHelper3_outParameterWriteIndirect_vh4(_uniforms, c);
    _skOutParamHelper4_inoutParameterWrite_vh4(c);
    _skOutParamHelper5_inoutParameterWriteIndirect_vh4(c);
    _out.sk_FragColor = c;
    return _out;
}
