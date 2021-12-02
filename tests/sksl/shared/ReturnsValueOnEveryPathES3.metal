#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    half unknownInput;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
bool inside_while_loop_b(Uniforms _uniforms) {
    while (_uniforms.unknownInput == 123.0h) {
        return false;
    }
    return true;
}
bool inside_infinite_do_loop_b() {
    do {
        return true;
    } while (true);
}
bool inside_infinite_while_loop_b() {
    while (true) {
        return true;
    }
}
bool after_do_loop_b() {
    do {
        break;
    } while (true);
    return true;
}
bool after_while_loop_b() {
    while (true) {
        break;
    }
    return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = (((inside_while_loop_b(_uniforms) && inside_infinite_do_loop_b()) && inside_infinite_while_loop_b()) && after_do_loop_b()) && after_while_loop_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
