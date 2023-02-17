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
int out_param_func2_ih(Uniforms _uniforms, thread half& v);
int _skOutParamHelper1_out_param_func2_ih(Uniforms _uniforms, thread array<half, 2>& testArray) {
    half _var0;
    int _skResult = out_param_func2_ih(_uniforms, _var0);
    testArray[0] = _var0;
    return _skResult;
}
int out_param_func2_ih(Uniforms _uniforms, thread half& v);
int _skOutParamHelper2_out_param_func2_ih(Uniforms _uniforms, thread array<half, 2>& testArray) {
    half _var0;
    int _skResult = out_param_func2_ih(_uniforms, _var0);
    testArray[0] = _var0;
    return _skResult;
}
void out_param_func1_vh(Uniforms _uniforms, thread half& v);
void _skOutParamHelper0_out_param_func1_vh(Uniforms _uniforms, thread array<half, 2>& testArray) {
    half _var0 = testArray[    _skOutParamHelper1_out_param_func2_ih(_uniforms, testArray)];
    out_param_func1_vh(_uniforms, _var0);
    testArray[    _skOutParamHelper2_out_param_func2_ih(_uniforms, testArray)] = _var0;
}
void out_param_func1_vh(Uniforms _uniforms, thread half& v) {
    v = _uniforms.colorGreen.y;
}
int out_param_func2_ih(Uniforms _uniforms, thread half& v) {
    v = _uniforms.colorRed.x;
    return int(v);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<half, 2> testArray;
    _skOutParamHelper0_out_param_func1_vh(_uniforms, testArray);
    _out.sk_FragColor = testArray[0] == 1.0h && testArray[1] == 1.0h ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
