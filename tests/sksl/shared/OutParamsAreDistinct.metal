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
bool out_params_are_distinct_bhh(thread half& x, thread half& y);
bool _skOutParamHelper0_out_params_are_distinct_bhh(thread half& x, thread half&) {
    half _var0;
    half _var1;
    bool _skResult = out_params_are_distinct_bhh(_var0, _var1);
    x = _var0;
    x = _var1;
    return _skResult;
}
bool out_params_are_distinct_bhh(thread half& x, thread half& y) {
    x = 1.0h;
    y = 2.0h;
    return x == 1.0h && y == 2.0h;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half x = 0.0h;
    _out.sk_FragColor =     _skOutParamHelper0_out_params_are_distinct_bhh(x, x) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
