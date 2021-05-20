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
bool test_return_b() {
    do {
        return true;
    } while (false);
}
bool test_break_b() {
    do {
        break;
    } while (false);
    return true;
}
bool test_continue_b() {
    do {
        continue;
    } while (false);
    return true;
}
bool test_if_return_b(Uniforms _uniforms) {
    do {
        if (_uniforms.colorGreen.y > 0.0) {
            return true;
        } else {
            break;
        }
        continue;
    } while (false);
    return false;
}
bool test_if_break_b(Uniforms _uniforms) {
    do {
        if (_uniforms.colorGreen.y > 0.0) {
            break;
        } else {
            continue;
        }
    } while (false);
    return true;
}
bool test_else_b(Uniforms _uniforms) {
    do {
        if (_uniforms.colorGreen.y == 0.0) {
            return false;
        } else {
            return true;
        }
    } while (false);
}
bool test_loop_return_b() {
    for (int x = 0;x < 0; ++x) {
        return false;
    }
    return true;
}
bool test_loop_break_b() {
    for (int x = 0;x <= 1; ++x) {
        break;
    }
    return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = ((((((test_return_b() && test_break_b()) && test_continue_b()) && test_if_return_b(_uniforms)) && test_if_break_b(_uniforms)) && test_else_b(_uniforms)) && test_loop_return_b()) && test_loop_break_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
