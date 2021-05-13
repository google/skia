#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
bool return_on_both_sides_b(Uniforms _uniforms) {
    if (_uniforms.unknownInput == 1.0) return true; else return true;
}
bool for_inside_body_b() {
    for (int x = 0;x <= 10; ++x) {
        return true;
    }
}
bool after_for_body_b() {
    for (int x = 0;x <= 10; ++x) {
        true;
    }
    return true;
}
bool for_with_double_sided_conditional_return_b(Uniforms _uniforms) {
    for (int x = 0;x <= 10; ++x) {
        if (_uniforms.unknownInput == 1.0) return true; else return true;
    }
}
bool if_else_chain_b(Uniforms _uniforms) {
    if (_uniforms.unknownInput == 1.0) return true; else if (_uniforms.unknownInput == 2.0) return false; else if (_uniforms.unknownInput == 3.0) return true; else if (_uniforms.unknownInput == 4.0) return false; else return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = ((((true && return_on_both_sides_b(_uniforms)) && for_inside_body_b()) && after_for_body_b()) && for_with_double_sided_conditional_return_b(_uniforms)) && if_else_chain_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
