#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
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
bool switch_with_all_returns_b(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            return true;
        case 2:
            return false;
        default:
            return false;
    }
}
bool switch_fallthrough_b(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            return true;
        case 2:
        default:
            return false;
    }
}
bool switch_fallthrough_twice_b(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
        case 2:
        default:
            return true;
    }
}
bool switch_with_break_in_loop_b(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                break;
            }
        default:
            return true;
    }
}
bool switch_with_continue_in_loop_b(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                continue;
            }
        default:
            return true;
    }
}
bool switch_with_if_that_returns_b(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            if (_uniforms.unknownInput == 123.0h) return false; else return true;
        default:
            return true;
    }
}
bool switch_with_one_sided_if_then_fallthrough_b(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            if (_uniforms.unknownInput == 123.0h) return false;
        default:
            return true;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = ((((((((((inside_while_loop_b(_uniforms) && inside_infinite_do_loop_b()) && inside_infinite_while_loop_b()) && after_do_loop_b()) && after_while_loop_b()) && switch_with_all_returns_b(_uniforms)) && switch_fallthrough_b(_uniforms)) && switch_fallthrough_twice_b(_uniforms)) && switch_with_break_in_loop_b(_uniforms)) && switch_with_continue_in_loop_b(_uniforms)) && switch_with_if_that_returns_b(_uniforms)) && switch_with_one_sided_if_then_fallthrough_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
