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
bool return_in_some_cases_bi(int x) {
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
bool return_with_fallthrough_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
        case 2:
            return true;
        default:
            break;
    }
    ++val;
    return val == 1;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int x = int(_uniforms.colorGreen.y);
    _out.sk_FragColor = ((((return_in_one_case_bi(x) && return_in_default_bi(x)) && return_in_every_case_bi(x)) && return_in_every_case_no_default_bi(x)) && return_in_some_cases_bi(x)) && return_with_fallthrough_bi(x) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
