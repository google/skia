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
bool TrueFalse_b() {
    int x = 1;
    int y = 1;
    if (x == 1 && (y += 1) == 3) {
        return false;
    } else {
        return x == 1 && y == 2;
    }
}
bool FalseTrue_b() {
    int x = 1;
    int y = 1;
    if (x == 2 && (y += 1) == 2) {
        return false;
    } else {
        return x == 1 && y == 1;
    }
}
bool FalseFalse_b() {
    int x = 1;
    int y = 1;
    if (x == 2 && (y += 1) == 3) {
        return false;
    } else {
        return x == 1 && y == 1;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_TrueTrue;
    int _2_y = 1;
    if ((_2_y += 1) == 2) {
        _0_TrueTrue = _2_y == 2;
    } else {
        _0_TrueTrue = false;
    }
    _out.sk_FragColor = ((_0_TrueTrue && TrueFalse_b()) && FalseTrue_b()) && FalseFalse_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
