#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
bool switch_fallthrough_twice_bi(int value) {
    bool ok = false;
    switch (value) {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
            ok = true;
            break;
        default:
            break;
    }
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int x = int(_uniforms.colorGreen.y);
    bool _0_ok = false;
    switch (x) {
        case 2:
            break;
        case 1:
        case 0:
            _0_ok = true;
            break;
        default:
            break;
    }
    _out.sk_FragColor = _0_ok && switch_fallthrough_twice_bi(x) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
