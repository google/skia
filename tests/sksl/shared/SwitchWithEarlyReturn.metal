#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
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
bool return_in_one_case_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            ++val;
            return false;
        default:
            ++val;
    }
    return val == 1;
}
bool return_in_default_bi(int x) {
    switch (x) {
        default:
            return true;
    }
}
bool return_in_every_case_bi(int x) {
    switch (x) {
        case 1:
            return false;
        default:
            return true;
    }
}
bool return_in_every_case_no_default_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            return false;
        case 2:
            return true;
    }
    ++val;
    return val == 1;
}
bool case_has_break_before_return_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            break;
        case 2:
            return true;
        default:
            return true;
    }
    ++val;
    return val == 1;
}
bool case_has_break_after_return_bi(int x) {
    switch (x) {
        case 1:
            return false;
        case 2:
            return true;
        default:
            return true;
    }
}
bool no_return_in_default_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            return false;
        case 2:
            return true;
        default:
            break;
    }
    ++val;
    return val == 1;
}
bool empty_default_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            return false;
        case 2:
            return true;
        default:
    }
    ++val;
    return val == 1;
}
bool return_with_fallthrough_bi(int x) {
    switch (x) {
        case 1:
        case 2:
            return true;
        default:
            return false;
    }
}
bool fallthrough_ends_in_break_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
        case 2:
            break;
        default:
            return false;
    }
    ++val;
    return val == 1;
}
bool fallthrough_to_default_with_break_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
        case 2:
        default:
            break;
    }
    ++val;
    return val == 1;
}
bool fallthrough_to_default_with_return_bi(int x) {
    switch (x) {
        case 1:
        case 2:
        default:
            return true;
    }
}
bool fallthrough_with_loop_break_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            for (int i = 0;i < 5; ++i) {
                ++val;
                break;
            }
        case 2:
        default:
            return true;
    }
}
bool fallthrough_with_loop_continue_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            for (int i = 0;i < 5; ++i) {
                ++val;
                continue;
            }
        case 2:
        default:
            return true;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int x = int(_uniforms.colorGreen.y);
    _out.sk_FragColor = ((((((((((((return_in_one_case_bi(x) && return_in_default_bi(x)) && return_in_every_case_bi(x)) && return_in_every_case_no_default_bi(x)) && case_has_break_before_return_bi(x)) && case_has_break_after_return_bi(x)) && no_return_in_default_bi(x)) && empty_default_bi(x)) && return_with_fallthrough_bi(x)) && fallthrough_ends_in_break_bi(x)) && fallthrough_to_default_with_break_bi(x)) && fallthrough_to_default_with_return_bi(x)) && fallthrough_with_loop_break_bi(x)) && fallthrough_with_loop_continue_bi(x) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
