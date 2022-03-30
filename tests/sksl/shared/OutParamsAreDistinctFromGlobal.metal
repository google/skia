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
struct Globals {
    half x;
};
bool out_params_are_distinct_from_global_bh(thread Globals& _globals, thread half& y);
bool _skOutParamHelper0_out_params_are_distinct_from_global_bh(thread Globals& _globals, thread half& x) {
    half _var0;
    bool _skResult = out_params_are_distinct_from_global_bh(_globals, _var0);
    x = _var0;
    return _skResult;
}
bool out_params_are_distinct_from_global_bh(thread Globals& _globals, thread half& y) {
    y = 2.0h;
    return _globals.x == 1.0h && y == 2.0h;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{1.0h};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor =     _skOutParamHelper0_out_params_are_distinct_from_global_bh(_globals, _globals.x) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
