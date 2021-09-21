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
bool switch_with_continue_in_loop_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            for (int i = 0;i < 10; ++i) {
                ++val;
                continue;
                ++val;
            }
        default:
            ++val;
    }
    return val == 11;
}
bool loop_with_break_in_switch_bi(int x) {
    int val = 0;
    for (int i = 0;i < 10; ++i) {
        switch (x) {
            case 1:
                ++val;
                break;
            default:
                return false;
        }
        ++val;
    }
    return val == 20;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int x = int(_uniforms.colorGreen.y);
    int _0_val = 0;
    switch (x) {
        case 1:
            for (int _1_i = 0;_1_i < 10; ++_1_i) {
                ++_0_val;
                break;
                ++_0_val;
            }
        default:
            ++_0_val;
    }
    _out.sk_FragColor = (_0_val == 2 && switch_with_continue_in_loop_bi(x)) && loop_with_break_in_switch_bi(x) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
