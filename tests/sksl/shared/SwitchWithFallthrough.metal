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
bool switch_fallthrough_bi(int value) {
    switch (value) {
        case 2:
            return false;
        case 1:
        case 0:
            return true;
        default:
            return false;
    }
}
bool switch_fallthrough_twice_bi(int value) {
    switch (value) {
        case 0:
            return false;
        case 1:
        case 2:
        case 3:
            return true;
        default:
            return false;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int x = int(_uniforms.colorGreen.y);
    _out.sk_FragColor = switch_fallthrough_bi(x) && switch_fallthrough_twice_bi(x) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
