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
    half _skTemp0;
    half _skTemp1;
    int _skTemp2;
    half _skTemp3;
    int _skTemp4;
    array<half, 2> testArray;
    ((out_param_func1_vh(_uniforms, (_skTemp0 = testArray[((_skTemp2 = out_param_func2_ih(_uniforms, _skTemp1)), (testArray[0] = _skTemp1), _skTemp2)]))), (testArray[((_skTemp4 = out_param_func2_ih(_uniforms, _skTemp3)), (testArray[0] = _skTemp3), _skTemp4)] = _skTemp0));
    _out.sk_FragColor = testArray[0] == 1.0h && testArray[1] == 1.0h ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
