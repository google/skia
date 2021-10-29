#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    int i;
};
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    int glob;
};
bool block_variable_hides_global_variable_b(thread Globals& _globals) {
    return _globals.glob == 2;
}
bool local_variable_hides_struct_b() {
    bool S = true;
    return S;
}
bool local_struct_variable_hides_struct_type_b() {
    S S = S{1};
    return S.i == 1;
}
bool local_variable_hides_global_variable_b() {
    int glob = 1;
    return glob == 1;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _globals.glob = 2;
    bool _0_var = true;
    _out.sk_FragColor = (((_0_var && block_variable_hides_global_variable_b(_globals)) && local_variable_hides_struct_b()) && local_struct_variable_hides_struct_type_b()) && local_variable_hides_global_variable_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
