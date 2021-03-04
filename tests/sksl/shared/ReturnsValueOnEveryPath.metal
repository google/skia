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



bool for_inside_body() {
    for (int x = 0;x <= 10; ++x) {
        return true;
    }
}
bool after_for_body() {
    for (int x = 0;x <= 10; ++x) {
    }
    return true;
}
bool for_with_double_sided_conditional_return(Uniforms _uniforms) {
    for (int x = 0;x <= 10; ++x) {
        if (_uniforms.unknownInput == 1.0) return true; else return true;
    }
}
bool switch_with_all_returns(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            return true;
        case 2:
            return true;
        default:
            return true;
    }
}
bool switch_only_default(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        default:
            return true;
    }
}
bool switch_fallthrough(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            return true;
        case 2:
        default:
            return true;
    }
}
bool switch_fallthrough_twice(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
        case 2:
        default:
            return true;
    }
}
bool switch_with_break_in_loop(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                break;
            }
        default:
            return true;
    }
}
bool switch_with_continue_in_loop(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                continue;
            }
        default:
            return true;
    }
}
bool switch_with_if_that_returns(Uniforms _uniforms) {
    switch (int(_uniforms.unknownInput)) {
        case 1:
            if (_uniforms.unknownInput == 123.0) return true; else return true;
        default:
            return true;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_return_on_both_sides;
    if (_uniforms.unknownInput == 1.0) _0_return_on_both_sides = true; else _0_return_on_both_sides = true;
    _out.sk_FragColor = (((((((((_0_return_on_both_sides && for_inside_body()) && after_for_body()) && for_with_double_sided_conditional_return(_uniforms)) && switch_with_all_returns(_uniforms)) && switch_only_default(_uniforms)) && switch_fallthrough(_uniforms)) && switch_fallthrough_twice(_uniforms)) && switch_with_break_in_loop(_uniforms)) && switch_with_continue_in_loop(_uniforms)) && switch_with_if_that_returns(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;


}
