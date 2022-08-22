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
    const int SEVEN;
    const int TEN;
};
bool verify_const_globals_bii(int seven, int ten) {
    return seven == 7 && ten == 10;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{7, 10};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = verify_const_globals_bii(_globals.SEVEN, _globals.TEN) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
