#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
constant const int SEVEN = 7;
constant const int TEN = 10;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
bool verify_const_globals_bii(int seven, int ten) {
    return seven == 7 && ten == 10;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = verify_const_globals_bii(SEVEN, TEN) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
